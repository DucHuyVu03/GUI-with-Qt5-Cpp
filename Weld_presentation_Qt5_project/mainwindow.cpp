#include "mainwindow.h"
#include "./ui_mainwindow.h"

QString getDataFolderPath() {
    return QCoreApplication::applicationDirPath() + "/data";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Change background
    QString sourcePath = QString(__FILE__); // full path to mainwindow.cpp
    QFileInfo fileInfo(sourcePath);
    QString sourceDir = fileInfo.absolutePath(); // path to the folder containing .cpp

    QString backgroundPath = sourceDir + "/helpers/background1.jpg";
    this->setStyleSheet(QString("QMainWindow {"
                                "background-image: url(%1);"
                                "background-repeat: no-repeat;"
                                "background-position: center;"
                                "}").arg(backgroundPath));
    //========== Folder ================================
    //Get the "data" folder
    QString dataContainingFolder = getDataFolderPath();
    qDebug()  << dataContainingFolder;
    QDir dir(dataContainingFolder);

    //Check if the folder exist
    if(!dir.exists())
    {
        qDebug() << "Data folder not exist!!!. Please create a folder name 'data'";
        return;
    }
    //==========Get all the files==============
    //Filter only the .jpg files
    QStringList imageFormatFilters;
    imageFormatFilters << "*.jpg" << "*.JPG";  // include both lowercase and uppercase
    QFileInfoList fileList = dir.entryInfoList(imageFormatFilters, QDir::Files | QDir::NoDotAndDotDot);

    //Display onto the list widget
    for (const QFileInfo &file : fileList) {
        ui->weldImageList->addItem(file.fileName());
    }
    //========================================================================

    //======== Folder refresh timer ===================
    folderWatcher = new QFileSystemWatcher(this);
    QString dataPath = getDataFolderPath();
    folderWatcher->addPath(dataPath);
    //========================================================================

    //================== S3 sync timer ============================
    QTimer *syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &MainWindow::sync_data_S3_to_local);
    syncTimer->start(1000);  // Every 1 second
    //================== S3 sync timer ============================

    //Connections
    connect(ui->weldImageList, &QListWidget::itemClicked,
            this, &MainWindow::on_fileItem_clicked);

    connect(ui->searchButton, &QPushButton::clicked,
            this, &MainWindow::on_searchButton_clicked);

    connect(folderWatcher, &QFileSystemWatcher::directoryChanged,
            this, &MainWindow::update_file_list);

    connect(ui->clearDataButton, &QPushButton::clicked,
            this, &MainWindow::on_clearDataButton_clicked,
            Qt::UniqueConnection);

    // Center the window within available screen space
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect availableGeometry = screen->availableGeometry();
    move(availableGeometry.center() - this->rect().center());

    //=== Start timer =====
    update_file_list();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchButton_clicked()
{
    QString searchText = ui->weldSearchTypeBox->text().trimmed();
    QString dataPath = getDataFolderPath();

    QDir dir(dataPath);
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG";

    QFileInfoList allFiles = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

    // Filter matching files
    QFileInfoList matchedFiles;
    for (const QFileInfo &file : allFiles) {
        if (file.fileName().contains(searchText, Qt::CaseInsensitive)) {
            matchedFiles.append(file);
        }
    }

    // Sort newest first
    std::sort(matchedFiles.begin(), matchedFiles.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified();
    });

    ui->weldImageList->clear();

    for (const QFileInfo &file : matchedFiles) {
        ui->weldImageList->addItem(file.fileName());
    }

    if (matchedFiles.isEmpty()) {
        ui->weldImageList->addItem("(No matches found)");
    }
}

void MainWindow::on_fileItem_clicked(QListWidgetItem *item) {
    if (!item) return;

    QString fileName = item->text();
    QString fullPath = getDataFolderPath() + "/" + fileName;

    QPixmap image(fullPath);
    if (image.isNull()) {
        QMessageBox::warning(this, "Image Load Error", "Failed to load image.");
        return;
    }

    // Scale 2× first
    QSize doubleSize = image.size() * 2;

    // Get QLabel size
    QSize labelSize = ui->weldImageLabel->size();

    // Determine final size: use 2× if it fits, otherwise fit to label
    QSize finalSize;
    if (doubleSize.width() <= labelSize.width() && doubleSize.height() <= labelSize.height()) {
        finalSize = doubleSize;
    } else {
        finalSize = labelSize;
    }

    QPixmap scaledImage = image.scaled(finalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Display
    ui->weldImageLabel->setAlignment(Qt::AlignCenter);
    ui->weldImageLabel->setPixmap(scaledImage);

    //Get the .txt file content corresponding to the name of the .jpg file
    QString baseName = QFileInfo(fileName).completeBaseName();  // "cat.jpg" -> "cat"
    QString textPath = getDataFolderPath() + "/" + baseName+ ".jpg" + ".txt";
    load_text_from_file(textPath);
}

void MainWindow::load_text_from_file(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Could not open text file.");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    ui->classNCommentLabel->setText(content);
}

void MainWindow::update_file_list() {
    QString dataPath = getDataFolderPath();
    QDir dir(dataPath);
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.JPG";

    QFileInfoList allFiles = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

    // Sort newest first
    std::sort(allFiles.begin(), allFiles.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified(); // descending
    });

    ui->weldImageList->clear();

    for (const QFileInfo &file : allFiles) {
        ui->weldImageList->addItem(file.fileName());
    }
}

void MainWindow::sync_data_S3_to_local() {
    QString folderPath = QCoreApplication::applicationDirPath() + "/data";
    QString s3Bucket = "s3://imageweld/results/";

    QStringList args;
    args << "s3" << "sync" << s3Bucket << folderPath;

    QProcess *process = new QProcess(this);

    // Stop and delete the process when it finishes
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            process, &QObject::deleteLater);

    // Optional: capture stdout and stderr for debugging
    connect(process, &QProcess::readyReadStandardOutput, [process]() {
        qDebug() << process->readAllStandardOutput();
    });
    connect(process, &QProcess::readyReadStandardError, [process]() {
        qDebug() << process->readAllStandardError();
    });

    // Register the process to be killed when app closes
    connect(qApp, &QCoreApplication::aboutToQuit, process, &QProcess::kill);

    qDebug() << "Executing: aws" << args;
    process->start("aws", args);
}

void MainWindow::on_clearDataButton_clicked() {
    QString folderPath = getDataFolderPath();
    QDir dir(folderPath);

    if (!dir.exists()) {
        QMessageBox::information(this, "Info", "Data folder does not exist.");
        return;
    }

    // Confirm with the user
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Deletion",
                                  "Are you sure you want to delete all files in the data folder?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    QStringList filters = {"*.jpg", "*.jpeg", "*.txt", "*.png"};  // Add more if needed
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

    int deletedCount = 0;
    for (const QFileInfo &fileInfo : fileList) {
        QFile::remove(fileInfo.absoluteFilePath());
        ++deletedCount;
    }

    update_file_list(); // Refresh the UI list
    QMessageBox::information(this, "Done", QString("Deleted %1 file(s).").arg(deletedCount));
}
