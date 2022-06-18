#include "mapeditor.h"
#include "ui_mapeditor.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFont>
#include <QFileDialog>
#include <QInputDialog>
#include <QBitmap>
#include <algorithm>
#include <QRegularExpression>

/************************定义全局变量************************/
QPoint P1,P2; // 操作时的临时边
QPoint P[51]; // 用于存储图中的每个节点
QString Pl[51], pic; // 每个点的标签，pic为当前地图的存储路径

bool showlen = false; // 是否显示当前路径长度

double INF = 1e9;
std::vector<int> path;
bool flag = false; // 显示最短路径

bool flag2 = false; // 显示查找地点
std::vector<int> finds; // 查找地点列表

std::vector<double> d(101, INF);  //源点到各个点的最短路径长度

struct lines{ // 图的边
    int a,b; // 两点p1，p2的下标
    int f = 0; // 是否为最短路径或最小树，是则标红，否为蓝色
    double len;
}temp1,line[101]; // temp1为当前边

int count = 0; // 添加的点的默认下标

/************************函数定义************************/

double MapEditor::dist(QPoint a, QPoint b){ //计算两点间的距离
    return sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()));
}

bool MapEditor::isin(QPoint x, QPoint y, int n){ // 判断鼠标是否成功点击(半径为n的圆域范围内）
    if(dist(x,y) <= n)
        return true;
    else
        return false;
}

void MapEditor::clear(){ // 清除路径显示栏中的信息
    flag = false;
    flag2 = false;
    ui->label_begin->setText("");
    ui->label_end->setText("");
    ui->listWidget->clear();
    ui->listWidget_2->clear();
    for(int i = 1; i <= e_num; i++){
        if(line[i].f)
            line[i].f = 0; // 红线变为蓝线
    }
    update(); // 调用paintEvent函数
}

/************************构造函数************************/
MapEditor::MapEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapEditor)
{
    ui->setupUi(this);

    //设置窗口无边框
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    //用绘图事件将窗口设置为圆角
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(), 20, 20);
    setMask(bmp);

    // 槽函数和信号
    connect(ui->pushButton_add,&QPushButton::clicked,this,&MapEditor::add_cliked);
    connect(ui->pushButton_delete,&QPushButton::clicked,this,&MapEditor::delete_cliked);
    connect(ui->pushButton_add_line,&QPushButton::clicked,this,&MapEditor::line_cliked);
    connect(ui->pushButton_delete_line,&QPushButton::clicked,this,&MapEditor::delete_line_cliked);
    connect(ui->pushButton_edit,&QPushButton::clicked,this,&MapEditor::edit_cliked);
    connect(ui->pushButton_edit_line,&QPushButton::clicked,this,&MapEditor::edit_length_cliked);
    connect(ui->pushButton_show,&QPushButton::clicked,this,&MapEditor::show_cliked);
    connect(ui->pushButton_select,&QPushButton::clicked,this,&MapEditor::select_cliked);
    connect(ui->pushButton_changebackground,&QPushButton::clicked,this,&MapEditor::changeBackground_cliked);
    connect(ui->pushButton_load,&QPushButton::clicked,this,&MapEditor::load_cliked);
    connect(ui->pushButton_tree,&QPushButton::clicked,this,&MapEditor::tree_cliked);
    connect(ui->pushButton_find,&QPushButton::clicked,this,&MapEditor::find_cliked);
    connect(ui->pushButton_save,&QPushButton::clicked,this,&MapEditor::save_cliked);
    connect(ui->pushButton_clear,&QPushButton::clicked,this,&MapEditor::clear_cliked);
    connect(ui->pushButton_edit_title,&QPushButton::clicked,this,&MapEditor::edit_title_cliked);
    connect(ui->pushButton_exit,&QPushButton::clicked,this,[&](){this->close();delete ui;});
    connect(ui->pushButton,&QPushButton::clicked,this,[&](){this->showMinimized();});

    ui->label->setText("等待一个操作");
    //使listwdiget依据宽度自动换行
    ui->listWidget->setWordWrap(true);
    ui->listWidget_2->setWordWrap(true);

    ui->label->setMargin(10);
    ui->label_e->setMargin(10);
    ui->label_v->setMargin(10);
    ui->label_begin->setMargin(10);
    ui->label_end->setMargin(10);
    ui->label_3->setMargin(10);

    pic = "";
    for(int i = 1; i <= v_num; i++){
        P[i] = P[0];
        Pl[i] = Pl[0];
        for(int j = i; j <= v_num; j++)
            matrix[i][j] = 0;
    }
    for(int i = 1; i <= e_num; i++)
        line[i] = line[0];
    doing_Type = v_num = e_num = count = 0;
    clear();
    ui->label->setText("");
}

MapEditor::~MapEditor(){
    delete ui;
}

/************************槽函数************************/

void MapEditor::paintEvent(QPaintEvent*){ // 绘图函数，将数据与看到的地图同步
    QPainter painter(this); // 画笔
    QPen Bluepen, Redpen;
    Bluepen.setWidth(5);
    Bluepen.setColor(Qt::blue);
    Redpen.setWidth(5);
    Redpen.setColor(Qt::red);
    painter.drawPixmap(200,100,1200,650,QPixmap(pic));
    QFont font1("Microsoft YaHei",13); // 字体说明，用于打印路径长和点的标签
    QFont font2("Microsoft YaHei",15);
    painter.drawRect(200,100,1200,650); // 画图窗口的大小
    painter.setFont(font1);
    painter.setRenderHint(QPainter::Antialiasing,true); // 使接下来的绘图更加平滑

    for(int i = 1; i <= e_num; i ++){
        if(!line[i].f){
           if(!flag)
                painter.setPen(Bluepen); // 设置画笔为蓝色
           else
                painter.setPen(QColor(0,0,0,0)); // 透明
        }
        else
            painter.setPen(Redpen);
        painter.drawLine(P[line[i].a], P[line[i].b]); // 两点连成线
    }

    painter.setPen(Qt::darkMagenta);

    if(showlen){ // 如果显示路径
        for(int i = 1; i < v_num; i ++){
            for(int j = i + 1; j <= v_num; j ++){
                if(matrix[i][j]){ // 若路径存在
                    int x1,y1;
                    x1 = (P[i].x() + P[j].x()) / 2 - 8; // 路径中央偏左
                    y1 = (P[i].y() + P[j].y()) / 2 - 3; // 路径中央偏上
                    painter.drawText(QPoint(x1,y1),QString::number(matrix[i][j])); // 显示路径长度在（x，y）处
                }
            }
        }
    }

    painter.setPen(Qt::black);
    painter.setBrush(Qt::yellow);
    painter.setFont(font2);

    for(int i = 1; i <= v_num; i++){
        painter.drawEllipse(P[i],5,5);//将节点用黄底黑边的圆画出来
        painter.drawText(QPoint(P[i].x() + 5,P[i].y() + 5),Pl[i]); // 画出点的标签，pl为标签，标签及节点的名称
    }

    if(flag2){
        for(auto& a : finds){
            painter.setBrush(Qt::red);
            painter.drawEllipse(P[a],10,10);
        }
    }

    ui->label_v->setText("地图节点数：" + QString::number(v_num));
    ui->label_e->setText("地图路径数：" + QString::number(e_num));
}

void MapEditor::mousePressEvent(QMouseEvent *event){ // 鼠标事件
    //只允许左键拖动   持续的动作
    if(event->buttons()&Qt::LeftButton && !(event->pos().y() < 750 && event->pos().y() > 100
                                            && event->pos().x() < 1400 && event->pos().x() > 200)){
        //求差值=鼠标当前位置-窗口左上角点
        m_pt = event->globalPos()-this->geometry().topLeft();  //geometry()是矩形窗口，topLeft()是左上角的信息。
    }
    if(event->button() == Qt::LeftButton && event->pos().y() < 750 && event->pos().y() > 100
            && event->pos().x() < 1400 && event->pos().x() > 200){
        QPoint temp = event->pos(); // event->pos为当前点击位置
        switch (doing_Type){
        case 1: // 选择第一个点
            if(e_num == 100)
                QMessageBox::warning(this,"警告","路径数已达上限");
            else{
                for(int i = 1; i <= v_num; i++)
                    if(isin(temp,P[i])){ // 选中
                        P1 = P[i];//存入P1
                        line[e_num + 1].a = i;//将该点录入line
                        doing_Type = 2;
                        ui->label->setText("请选择第二个点");
                        break;
                    }
            }
            break;
        case 2: // 选择第二个点
            for (int i = 1; i <= v_num; i++){
                if(P[i] != P1 && isin(temp,P[i])){ // 若选中了与第一个点不同的点
                    int t = e_num++; // 边数量+1
                    P2 = P[i]; // 录入P2
                    line[e_num].b = i; // 将该点录入line
                    doing_Type = 1;
                    if(line[e_num].a > line[e_num].b){ // 保证边的第一个点的下标比第二个的小
                        int t1 = line[e_num].a;
                        line[e_num].a = line[e_num].b;
                        line[e_num].b = t1;
                    }
                    for(int j = 1; j < e_num; j++){ // 判断是否路线已存在
                        if(line[e_num].a == line[j].a && line[e_num].b == line[j].b){
                            line[e_num--] = line[0];
                            QMessageBox::warning(this,"警告","两条路径重合");
                            break;
                        }
                    }
                    if(t != e_num){ // 路径添加成功将两点间的像素距离赋值给两点间的路径长度（默认）
                        matrix[line[e_num].a][line[e_num].b] = matrix[line[e_num].b][line[e_num].a]
                                = dist(P[line[e_num].a],P[line[e_num].b]);
                    }
                    ui->label->setText("请选择第一个点");
                    break;
                }
            }
            update();
            break;
        case 3: // 添加点
            if(v_num < 50 && temp.x() >= 200 && temp.x() <= 1400 && temp.y() > 100 && temp.y() <= 750){ // 判断所加的点是否在窗口范围内
                int t=v_num++;
                for(int i = 1; i < v_num; i++){
                    if(isin(temp,P[i],20)){ // 判断两点是否太近，两点间距大于20个像素
                        v_num--;
                        QMessageBox::warning(this,"警告","两点相距太近");
                    }
                }
                if(t == v_num)
                    break;
                P[v_num] = event->pos(); // 将当前位置赋给新的P点
                Pl[v_num] = QString::number(++count); // 创建默认标签
                update();
            }
            else if(v_num == 50)
                QMessageBox::warning(this,"警告","点数已达上限");
            else
                QMessageBox::warning(this,"警告","点超出边界");
            break;
        case 4: // 删除点
            if(v_num == 0)
                QMessageBox::warning(this,"警告","无任何点");
            else{
                for(int i = 1; i <= v_num; i++){
                    if(isin(temp,P[i])){ // 选中想要删除的点
                        for(int j = i; j < v_num; j++){
                            P[j] = P[j + 1]; // 将删除的点之后的点前移
                            Pl[j] = Pl[j + 1]; // 标签同理
                            for (int k = 1; k <= v_num; k++){ // 删除该点对应下标的行和列
                                matrix[j][k] = matrix[j + 1][k];
                            }
                        }
                        for(int j = i;j < v_num; j++){
                            for(int k = 1 ;k <= v_num; k++){
                                matrix[k][j] = matrix[k][j + 1];
                            }
                        }
                        for(int j = 1; j <= v_num; j++){
                            matrix[j][v_num]=matrix[v_num][j]=0;
                        }
                        Pl[v_num] = Pl[0]; // 最后一个点的标签消失
                        P[v_num--] = P[0]; // 最后一个点消失
                        for(int j = 1; j <= e_num; j++){
                            if(i == line[j].a || i == line[j].b){ // 将line数组一并前移
                                for(int k = j; k < e_num; k++)
                                    line[k] = line[k + 1];
                                line[e_num--] = line[0]; // 将最后一条边消除，同时边数量减一
                                j--;
                            }
                            else{ // 下标在被选中的点所边的点的下标前移
                                if(line[j].a > i)
                                    line[j].a--;
                                if(line[j].b > i)
                                    line[j].b--;
                            }
                        }
                        update();
                        break;
                    }
                }
            }
            break;
        case 5: // 选择第一个点删除的路径
            if(e_num == 0)
                QMessageBox::warning(this,"警告","无任何路径");
            else{
                for(int i = 1; i <= v_num; i++){
                    if(isin(temp,P[i])){ // 判断选中
                        P1 = P[i];
                        temp1.a = i;
                        doing_Type = 6;
                        ui->label->setText("请选择要删除路径的第二个点");
                    }
                }
            }
            break;
        case 6: // 选择第二个点删除路径
            for(int i = 1; i <= v_num; i++){
                if(P[i] != P1 && isin(temp,P[i])){ // 选中
                    P2 = P[i];
                    temp1.b = i;
                    doing_Type = 5; // 载入第二个点的信息
                    if(temp1.a > temp1.b){ // 保证线的第二个点下标大于第一个
                        int t1 = temp1.a;
                        temp1.a = temp1.b;
                        temp1.b = t1;
                    }
                    int t3 = e_num;
                    for(int j = 1; j <= e_num; j++){
                        if(temp1.a == line[j].a && temp1.b == line[j].b){
                            matrix[line[j].a][line[j].b] = matrix[line[j].b][line[j].a] = 0;//删除点之间的路径长度

                            for(int k = j ;k < e_num; k++)
                                line[k] = line[k + 1];//路径下标前移
                            line[e_num--] = line[0];
                            break;

                        }
                    }
                    ui->label->setText("请选择要删除路径的第一个点");
                    if(e_num == t3)
                        QMessageBox::warning(this,"警告","找不到这条路径");
                    else
                        break;
                }
            }
            update();
            break;
        case 7: // 编辑点的标签
            if(v_num == 0)
                QMessageBox::warning(this,"警告","无任何点");
            else{
                for(int i = 1; i <= v_num; i++){
                    if(isin(temp,P[i])){
                        QString s0=QInputDialog::getText(this,"编辑标签","输入文本（最多13个字");
                        if(s0 != "")
                            Pl[i] = s0.left(13);
                        break;
                    }
                }
            }
            break;
        case 8: // 选择起点
            for(int i = 1; i <= v_num; i ++){
                if(isin(temp,P[i])){
                    P1 = P[i];
                    doing_Type = 9;
                    ui->label->setText("请选择起点");
                    temp1.a = i; // 起点下标赋给temp1
                    ui->label_begin->setText("起点：" + Pl[i]);
                    ui->label->setText("请选择终点");
                    ui->label_end->setText("终点：");
                    break;
                }
            }
            break;
        case 9: // 选择终点
            for(int i = 1; i <= v_num; i++){
                if(P[i] != P1 && isin(temp,P[i])){ // 判断是否与起点重
                    P2 = P[i];
                    temp1.b = i; // 终点下标赋给temp1
                    ui->label_end->setText("终点：" + Pl[i]);
                    ui->listWidget->clear();
                    Dijkstra(temp1.a);
                    if(d[temp1.b] < INF){
                        ui->label_3->setText("存在最短路径");

                        int count = 0;
                        int temp2[51];

                        for(int j = 1; j <= e_num; j ++){
                            for(int i = temp1.b; path[i] != i; i = path[i]){
                                if((line[j].a == i && line[j].b == path[i]) || (line[j].b == i && line[j].a == path[i])){
                                    line[j].f = true;
                                    break;
                                }
                            }
                        }

                        for(int i = temp1.b; path[i] != i; i = path[i])
                            temp2[count ++] = i;

                        QString context = "最短路径为: " + QString::number(temp1.a);
                        for(int i = count - 1; i >= 0; i --)
                            context += "->" + QString::number(temp2[i]);
                        ui->listWidget_2->addItem(context + "\n" + "长度为：" + QString::number(d[temp1.b]));
                        update();
                    }
                    else
                        ui->label_3->setText("找不到路径");

                    doing_Type = 0;
                    ui->label->setText("");
                    break;
                }
            }
            break;
        case 10: // 编辑路径长度第一个点
            if(e_num == 0)
                QMessageBox::warning(this,"警告","无任何路径");
            else{
                for(int i = 1; i <= v_num; i++)
                    if(isin(temp,P[i])){ // 选中
                        P1 = P[i];
                        doing_Type = 11;
                        ui->label->setText("请选择要编辑路径长度的第二个点"); // 转入下个case
                        temp1.a = i;
                        break;
                    }
            }
            break;
        case 11: // 编辑路径长度第二个点
            for(int i = 1; i <= v_num; i++){
                if(P[i] != P1 && isin(temp,P[i])){ // 不与第一个点重合
                    P2 = P[i];
                    temp1.b = i;
                    doing_Type = 10;
                    if(temp1.a > temp1.b){
                        int t1 = temp1.a;
                        temp1.a = temp1.b;
                        temp1.b = t1;
                    }
                    bool f0 = false;
                    for(int j = 1; j <= e_num; j++){
                        if(temp1.a == line[j].a && temp1.b == line[j].b){
                            double number = QInputDialog::getDouble(this,"编辑长度","输入浮点数（0.0001~999999",matrix[temp1.a][temp1.b]);
                            if(number) // 若输入了数，则更改点之间的长度
                                matrix[temp1.a][temp1.b] = matrix[temp1.b][temp1.a] = number;
                            f0 = true;
                            break;
                        }
                    }
                    ui->label->setText("请选择要编辑路径长度的第一个点");
                    if(!f0)
                        QMessageBox::warning(this,"警告","找不到这条路径");
                    update();
                    break;
                }
            }
            break;
        }
    }
}

void MapEditor::mouseMoveEvent(QMouseEvent *event){
    // 只允许左键拖动,持续的动作
    if(event->buttons()&Qt::LeftButton && !(event->pos().y() < 750 && event->pos().y() > 100
                                            && event->pos().x() < 1400 && event->pos().x() > 200)){  // buttons处理的是长事件，button处理的是短暂的事件
        // 窗口跟随鼠标移动
        // 窗口的新位置=鼠标当前位置-差值
        move(event->globalPos() - m_pt);

    }
}

void MapEditor::add_cliked(){ // 添加点
    clear();
    if(doing_Type != 3){
        doing_Type = 3;
        ui->label->setText("请选择一个位置添加点");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::line_cliked(){ // 连接路径
    clear();
    if(doing_Type != 1){
        doing_Type = 1;
        ui->label->setText("请选择第一个点");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::delete_cliked(){ // 删除点
    clear();
    if(doing_Type != 4){
        doing_Type = 4;
        ui->label->setText("请选择一个点进行删除");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::delete_line_cliked(){ // 删除路线
    clear();
    if(doing_Type != 5){
        doing_Type = 5;
        ui->label->setText("请选择一个要删除路径的第一个点");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::edit_cliked(){ // 编辑点的标签
    clear();
    if(doing_Type != 7){
        doing_Type = 7;
        ui->label->setText("请选择要编辑地名的点");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::select_cliked(){ // 寻找路径
    clear();
    if(doing_Type != 8){
        if(e_num == 0){ // 判断地图是否存在
            QMessageBox::warning(this,"警告","无任何路径");
            return;
        }
        doing_Type = 8;
        ui->label->setText("请选择起点");
        ui->label_begin->setText("起点：");
        ui->listWidget->clear();
        ui->label_end->setText("");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::show_cliked(){ // 显示路径长
    clear();
    if(!showlen){
        showlen = true;
        ui->pushButton_show->setText("隐藏路径长");
    }
    else{
       showlen = false;
       ui->pushButton_show->setText("显示路径长"); // 每点一下切换状态
    }
    update();
}

void MapEditor::edit_length_cliked(){ // 设置路径长度
    clear();
    if(doing_Type != 10){
        doing_Type = 10;
        ui->label->setText("请选择要编辑路径长度的第一个点");
    }
    else{
        doing_Type = 0;
        ui->label->setText(""); // 跳出
    }
}

void MapEditor::tree_cliked(){
    clear();

    ui->label->setText("最小生成树");
    double value = minTree();
    if((value + 1.0) <= 1e-5)
        ui->label_3->setText("最小生成树不存在");
    else{
        ui->label_3->setText("最小生成树存在");
        ui->listWidget_2->addItem("生成树的总权值为" + QString::number(value,'f',2));
        update();
    }

}

void MapEditor::find_cliked(){
    clear();

    ui->label->setText("地点查找");
    QString dlgTitle = "地点搜索";
    QString txtLabel = "请输入地点名";
    QString defaultInput = "北京邮电大学";
    QLineEdit::EchoMode echoMode = QLineEdit::Normal; // 正常文字输入

    bool ok = false;
    QString text = QInputDialog::getText(this,dlgTitle,txtLabel,echoMode,defaultInput,&ok);
    if (ok && !text.isEmpty()){
        int f = false;

        finds.clear();
        finds.resize(51);

        flag2 = true;
        QRegularExpression re(".*" + text + ".*");
        for(int i = 1; i <= v_num; i ++){
            QRegularExpressionMatch match = re.match(Pl[i]);
            if(match.hasMatch()){
                f = true;
                finds.push_back(i);
                QString matched = match.captured(0);
                ui->listWidget->addItem(Pl[i]);
            }
        }

        if(f)
             ui->label_3->setText("查找成功");
        else
             ui->label_3->setText("查找失败");
    }
    else
        ui->label_3->setText("查找请求失败");
}

void MapEditor::edit_title_cliked(){ // 设置标题
    QString s0 = QInputDialog::getText(this,"编辑标题","输入文本（最多28个字",QLineEdit::Normal,ui->label_edit_title->text());
    if(s0 != "") ui->label_edit_title->setText(s0.left(28));
}

void MapEditor::clear_cliked(){ // 清除文字框
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this,"提示","确认要清除地图吗？",QMessageBox::Yes | QMessageBox::No);
    if(btn == QMessageBox::Yes){
        pic = "";
        for(int i = 1; i <= v_num; i++)
        {
            P[i] = P[0];
            Pl[i] = Pl[0];
            for(int j = i;j <= v_num; j++)
                matrix[i][j]=0;
        }
        for(int i = 1; i <= e_num; i++)
            line[i] = line[0];
        doing_Type = v_num=e_num = count = 0;
        clear();
        ui->label->setText("");
    }
}

void MapEditor::changeBackground_cliked(){ // 更换背景图
    QString curPath = QDir::currentPath();
    QString temp = QFileDialog::getOpenFileName(this,"选择文件",curPath,"图像文件(*.jpeg *.jpg *.bmg *.png *.gif)");
    if(temp != "")
        pic = temp;//转入图片存储路径
    update();
}

void MapEditor::save_cliked(){ // 保存地图
    using namespace std;
    QString curPath = QDir::currentPath();
    QString filename = QFileDialog::getSaveFileName(this,"保存文件",curPath,"地图文件(*.map2)");
    if(filename != ""){ // 判断地址是否输入成功
        ofstream out(filename.toStdString());//转换为标准字符串进行保存
        if(out.is_open()){
            out << ui->label_edit_title->text().toStdString() << endl;
            out <<e_num << ' ' << v_num << endl;
            for(int i = 1;i <= v_num; i++)
                out << P[i].x() << ' '<<P[i].y() << ' ' << Pl[i].toStdString() << endl;
            for(int i = 1;i <= v_num; i++){
                for(int j = 1; j <= v_num; j++)
                    out << matrix[i][j] << ' ';
                out << endl;
            }
            for(int i = 1; i <= e_num; i++)
                out << line[i].a << ' ' << line[i].b << endl;
            out << pic.toStdString() << endl;
            QMessageBox::information(this,"提示","保存成功");
            out.close();
        }
        else
            QMessageBox::information(this,"提示","保存失败");
    }
}

void MapEditor::load_cliked(){ // 载入地图
    using namespace std;
    QString curPath = QDir::currentPath();
    QString filename = QFileDialog::getOpenFileName(this,"选择文件",curPath,"地图文件(*.map2)");
    if(filename != ""){ //判断地址是否输入成功
        ifstream in(filename.toStdString());
        if(in.is_open()){
            string c;
            in >> c;
            ui->label_edit_title->setText(QString::fromStdString(c)); // 转变为Qstring
            in >> e_num >> v_num;
            for(int i = 1; i <= v_num; i++){
                int a,b;
                in >> a >> b;
                P[i].setX(a);
                P[i].setY(b);
                in >> c;
                Pl[i] = QString::fromStdString(c);
            }
            for(int i = 1; i <= v_num; i++)
                for(int j = 1; j <= v_num; j++)
                    in >> matrix[i][j];
            for(int i = 1; i <= e_num; i++)
                in >> line[i].a >> line[i].b;
            in >> c;
            pic = QString::fromStdString(c);
            clear();
            QMessageBox::information(this,"提示","读取成功");
            in.close();
        }
        else
            QMessageBox::information(this,"提示","读取失败");
    }
}

void MapEditor::Dijkstra(int s){
    path.clear();   //每次运行前先清空并重置
    path.resize(51);
    memset(reach,0,sizeof(reach));
    d.assign(101,INF);

    d[s] = 0.0, path[s] = s;
    for(int i = 0; i <= v_num; i++){
        int u = -1;
        double mn = INF;
        for(int j = 1; j <= v_num; j++){  //找一个距离源点最近的未访问点
            if(!reach[j] && d[j] < mn){
                u = j;
                mn = d[j];
            }
        }
        if(u == -1) //剩下的点不与源点连通，结束
            return;

        reach[u] = 1;
        for(int v = 1; v <= v_num; v++){  //遍历其每条出边，更新从该点出发到目标点的最短距离
            if(!reach[v] && matrix[u][v] != 0 && d[u] + matrix[u][v] < d[v]){
                d[v] = d[u] + matrix[u][v];
                path[v] = u;
            }
        }
    }
}

double MapEditor::minTree(){
    for(int i = 1; i <= e_num; i ++)
        line[i].len = matrix[line[i].a][line[i].b];
    std::sort(line + 1, line + 1 + e_num, [](struct lines x, struct lines y){return x.len < y.len;});

    int tag[51];
    for(int i = 1; i <= e_num; i ++)
        tag[i] = i;

    double sum = 0;
    int count = 0;
    for(int i = 1; i <= e_num; i ++){
        if(tag[line[i].a] != tag[line[i].b]){
            count ++;
            sum += line[i].len;
            line[i].f = true;
            int tags = tag[line[i].a];

            for(int j = 1; j <= v_num; j ++){
                if(tags == tag[j])
                    tag[j] = tag[line[i].b];
            }
        }
    }
    if(count != v_num - 1)
        return -1.0;
    else
        return sum;
}
