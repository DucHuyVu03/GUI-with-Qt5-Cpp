#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QCoreApplication>
#include <QString>

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

    //Get all the files
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    //Display onto the list widget
    for (const QFileInfo &file : fileList) {
        ui->weldImageList->addItem(file.fileName());
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchButton_clicked()
{

}

