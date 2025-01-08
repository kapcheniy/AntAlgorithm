#ifndef ROADMAP_H
#define ROADMAP_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <vector>
#include <utility>
#include <QTextEdit>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QInputDialog>
#include <QSpinBox>
#include <QFormLayout>

class RoadMap : public QMainWindow {
    Q_OBJECT

public:
    explicit RoadMap(QWidget *parent = nullptr);
    void generateRandomMap(int nodeCount, int edgeCount);
    void findOptimalPath();

private:
    QWidget *helpWindow;
    QGraphicsScene *scene;
    std::vector<std::pair<int, int>> nodes;
    std::vector<std::tuple<int, int, double>> edges;
    QVector<QVector<double>> adjacencyMatrix;
    void drawBestPathLength(double length);
    void drawNoPath();
    void drawMap();
    void createGraph();
    void setGraphParameters();
    bool isPressed = true;
};

#endif
