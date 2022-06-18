#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapeditor.h"
#include "mydialog.h"
#include <QMap>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //开始界面
    void on_loadButton_clicked();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void read();
    void write();

private slots:
    void on_registerButton_clicked();
    void setAccount(QString user,QString pwsd);


private:
    QPoint off;
    Ui::MainWindow *ui;
    MapEditor *x;
    QMap<QString, QString> Qmap;
};
#endif // MAINWINDOW_H
