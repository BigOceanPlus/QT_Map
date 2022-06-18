#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <QDialog>
#include <cmath>
#include <QMainWindow>
#include <fstream>

namespace Ui {
class MapEditor;
}

class MapEditor : public QDialog{
    Q_OBJECT

public:
    explicit MapEditor(QWidget *parent = nullptr);

    int doing_Type = 0; // 表示目前执行的操作状态
    int e_num, v_num; // 当前路径数，点数
    double matrix[51][51] = {}; // 图的邻接矩阵
    bool reach[51] = {}; // 用于存储已经过的点，避免重复使用

    void DFS(int P1,int P2); // 深度优先算法
    bool isin(QPoint x,QPoint y,int n = 10); // 用于判断鼠标点击的位置是否选中了图的某个点
    double dist(QPoint a,QPoint b); // 计算两点间的像素距离，等效于地图的实际距离
    void clear(); // 显示搜索的数据

    ~MapEditor();

    //槽函数
    void add_cliked();
    void line_cliked();
    void delete_cliked();
    void delete_line_cliked();
    void edit_cliked();
    void select_cliked();
    void show_cliked();
    void edit_length_cliked();
    void edit_title_cliked();
    void clear_cliked();
    void changeBackground_cliked();
    void save_cliked();
    void load_cliked();
    void tree_cliked();
    void find_cliked();

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void Dijkstra(int s);
    double minTree();  // 最小生成树


private:
    Ui::MapEditor *ui;
    QPoint m_pt;
};

#endif // MAPEDITOR_H
