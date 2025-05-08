#include "mainwindow.h"
#include "./ui_mainwindow.h"

QString getDataFolderPath() {
    QString sourcePath = QString(__FILE__);  // Path to mainwindow.cpp
    QDir dir = QFileInfo(sourcePath).dir();  // Get directory of mainwindow.cpp
    return dir.filePath("data");             // Append "data" folder
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
    //Get all the files==============
    //Filter only the .jpg files
    QStringList imageFormatFilters;
    imageFormatFilters << "*.jpg" << "*.JPG";  // include both lowercase and uppercase
    QFileInfoList fileList = dir.entryInfoList(imageFormatFilters, QDir::Files | QDir::NoDotAndDotDot);

    //Display onto the list widget
    for (const QFileInfo &file : fileList) {
        ui->weldImageList->addItem(file.fileName());
    }
    //========================================================================

    //======== Refresh timer ===================
    folderWatcher = new QFileSystemWatcher(this);
    QString dataPath = getDataFolderPath();
    folderWatcher->addPath(dataPath);
    //========================================================================

    //Connection
    connect(ui->weldImageList, &QListWidget::itemClicked,
            this, &MainWindow::on_fileItem_clicked);

    connect(ui->searchButton, &QPushButton::clicked,
            this, &MainWindow::on_searchButton_clicked);

    connect(folderWatcher, &QFileSystemWatcher::directoryChanged,
            this, &MainWindow::update_file_list);
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
    nameFilters << "*.jpg" << "*.JPG";

    QFileInfoList allFiles = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

    ui->weldImageList->clear();

    for (const QFileInfo &file : allFiles) {
        if (file.fileName().contains(searchText, Qt::CaseInsensitive)) {
            ui->weldImageList->addItem(file.fileName());
        }
    }

    if (ui->weldImageList->count() == 0) {
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

    ui->weldImageLabel->setPixmap(image);
    //Get the .txt file content corresponding to the name of the .jpg file
    QString baseName = QFileInfo(fileName).completeBaseName();  // "cat.jpg" -> "cat"
    QString textPath = getDataFolderPath() + "/" + baseName + ".txt";
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

    ui->weldImageList->clear();

    for (const QFileInfo &file : allFiles) {
        ui->weldImageList->addItem(file.fileName());
    }
}
