#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QCoreApplication>
#include <QString>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchButton_clicked();
    void on_fileItem_clicked(QListWidgetItem *item);
    void load_text_from_file(const QString &filePath);
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
