#include "mydialog.h"
#include "ui_mydialog.h"
#include<QPushButton>
MyDialog::MyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MyDialog)
{
    ui->setupUi(this);

}

MyDialog::~MyDialog()
{
    delete ui;
}


void MyDialog::on_pushButton1_clicked()
{
    emit request(ui->lineEdit->text(),ui->lineEdit_2->text());
    this->accept();
}
