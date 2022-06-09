#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>

namespace Ui {
class MyDialog;
}

class MyDialog : public QDialog
{
    Q_OBJECT

signals:
    void request(QString user,QString pswd);
public:
    explicit MyDialog(QWidget *parent = nullptr);
    ~MyDialog();

private slots:
    void on_pushButton1_clicked();
private:
    Ui::MyDialog *ui;
};

#endif // MYDIALOG_H
