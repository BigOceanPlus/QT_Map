#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mydialog.h"
#include <QBitmap>
#include <QPainter>
#include <QMouseEvent>
#include <QMap>
#include <QMessageBox>
#include <synchapi.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    read();
    //ui->statusbar->addWidget(label);

    ui->setupUi(this);
    //设置窗口无边框
    setWindowFlags (Qt::FramelessWindowHint);
    //用绘图事件将窗口设置为圆角
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(), 20, 20);
    setMask(bmp);

    ui->line2->setEchoMode(QLineEdit::Password); //密码输入编辑框设置为密码输入模式


    //将进入的按钮点击信号clicked与槽函数on_loadButton_clicked相连接
    connect(ui->loadButton,&QPushButton::clicked,this,&MainWindow::on_loadButton_clicked);
    connect(ui->cancelButton,&QPushButton::clicked,this,[=](){this->close();delete ui;});
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadButton_clicked()
{
    if(Qmap.contains(ui->line1->text()) && ui->line2->text() == Qmap[ui->line1->text()]){

        x=new MapEditor;
        x->show();


        write();
        Sleep(1000);
        this->close();
    }
    else{
        QString dlgTitle = "warning";
        QString strInfo = "账户密码错误";
        QMessageBox::warning(this, dlgTitle, strInfo);
    }

}

void MainWindow::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        QPoint startPos = event->globalPos();
        off = startPos - pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() == Qt::LeftButton){
        QPoint endPos = event->globalPos();
        move(endPos - off);
    }
}

void MainWindow::on_registerButton_clicked()
{
    MyDialog* dlgLocate;
    dlgLocate = new MyDialog(this);
    dlgLocate->setAttribute(Qt::WA_DeleteOnClose); //对话框关闭时自动删除对话框对象,用于不需要读取返回值的对话框
    Qt::WindowFlags flags = dlgLocate->windowFlags(); //获取已有flags
    dlgLocate->setWindowFlags(flags | Qt::WindowStaysOnTopHint); //设置对话框固定大小,StayOnTop

    connect(dlgLocate,SIGNAL(request(QString,QString)),
            this,SLOT(setAccount(QString,QString)));

    dlgLocate->show(); //非模态显示对话框
}

void MainWindow::read(){
    QString aFileName = "data.dat";
    QFile aFile(aFileName);  //以文件方式读出
    if (!(aFile.open(QIODevice::ReadOnly)))
        return;

    QDataStream aStream(&aFile); //用文本流读取文件
    aStream.setByteOrder(QDataStream::LittleEndian);

    int count;
     aStream.readRawData((char *)&count, sizeof(int));

    char* buf;
    uint strLen;
    QString user,pwsd;
    for(int i = 0; i < count; i ++){
        aStream.readBytes(buf,strLen);
        user = QString::fromLocal8Bit(buf,strLen);
        aStream.readBytes(buf,strLen);
        pwsd = QString::fromLocal8Bit(buf,strLen);
        Qmap.insert(user,pwsd);
    }
    aFile.close();
}

void MainWindow::write(){
    QString aFileName = "data.dat";
    QFile aFile(aFileName);  //以文件方式读出
    if (!(aFile.open(QIODevice::WriteOnly)))
        return;

    QDataStream aStream(&aFile); //用文本流读取文件
    aStream.setByteOrder(QDataStream::LittleEndian);

    int count = Qmap.size();
    aStream.writeRawData((char *)&count,sizeof(int)); //写入文件流

    QMap<QString, QString>::const_iterator it = Qmap.constBegin();
    while(it != Qmap.constEnd()){
        QByteArray btArray1 = it.key().toUtf8();
        aStream.writeBytes(btArray1,btArray1.length());
        QByteArray btArray2 = it.value().toUtf8();
        aStream.writeBytes(btArray2,btArray2.length());
        it ++;
    }
    aFile.close();
}

void MainWindow::setAccount(QString user,QString pwsd){
    if(Qmap.contains(user)){
        ui->Mylabel->setText("注册账号失败,账号已存在");
    }
    else{
        Qmap.insert(user,pwsd);
        ui->Mylabel->setText("成功注册账号");
    }
}

