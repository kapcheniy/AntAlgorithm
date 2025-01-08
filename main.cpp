#include "roadmap.h"
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QRandomGenerator>
#include <cmath>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RoadMap roadmap;
    roadmap.resize(800, 600);
    roadmap.generateRandomMap(5, 20);
    roadmap.show();
    roadmap.findOptimalPath();
    return app.exec();
}
