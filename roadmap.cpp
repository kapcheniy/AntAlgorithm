#include "roadmap.h"
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QRandomGenerator>
#include <cmath>

const int NUM_ANTS = 20;         // Количество муравьев
const int NUM_ITERATIONS = 100;  // Количество итераций для алгоритма муравьиной колонии
const double ALPHA = 1.0;        // Влияние феромонов при расчете вероятности перехода
const double BETA = 2.0;         // Влияние расстояния при расчете вероятности перехода
const double EVAPORATION = 0.5;  // Коэффициент испарения феромонов
const double Q = 100.0;          // Константа для обновления феромонов

RoadMap::RoadMap(QWidget *parent)
    : QMainWindow(parent), scene(new QGraphicsScene(this)) {
    QGraphicsView *view = new QGraphicsView(scene, this);
    setCentralWidget(view);

    QToolBar *toolBar = addToolBar("Tool Bar");

    QAction *createGraphAction = new QAction("Создать граф", this);
    toolBar->addAction(createGraphAction);
    connect(createGraphAction, &QAction::triggered, this, &RoadMap::createGraph);

    QAction *findPath = new QAction("Найти лучший маршрут", this);
    toolBar->addAction(findPath);
    connect(findPath, &QAction::triggered, this, &RoadMap::findOptimalPath);

    QAction *setGraphParameterAction = new QAction("Выбрать количество вершин или рёбер", this);
    toolBar->addAction(setGraphParameterAction);
    connect(setGraphParameterAction, &QAction::triggered, this, &RoadMap::setGraphParameters);
}

void RoadMap::createGraph() {
    int nodeCount = nodes.size();
    int edgeCount = edges.size();
    generateRandomMap(nodeCount, edgeCount);
    if (helpWindow->isActiveWindow()) {
        helpWindow->close();
    }
    helpWindow->close();
}

void RoadMap::setGraphParameters() {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Настройка параметров графа");

    QFormLayout *formLayout = new QFormLayout(dialog);

    QSpinBox *nodeSpinBox = new QSpinBox(dialog);
    nodeSpinBox->setRange(1, 100);
    nodeSpinBox->setValue(nodes.size());
    formLayout->addRow("Количество вершин:", nodeSpinBox);

    QSpinBox *edgeSpinBox = new QSpinBox(dialog);
    edgeSpinBox->setRange(1, 100);
    edgeSpinBox->setValue(edges.size());
    formLayout->addRow("Количество рёбер:", edgeSpinBox);

    QPushButton *okButton = new QPushButton("ОК", dialog);
    formLayout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);

    if (dialog->exec() == QDialog::Accepted) {
        int nodeCount = nodeSpinBox->value();
        int edgeCount = edgeSpinBox->value();
        generateRandomMap(nodeCount, edgeCount);
    }
    if (helpWindow->isActiveWindow()) {
        helpWindow->close();
    }
    delete dialog;
}

void RoadMap::generateRandomMap(int nodeCount, int edgeCount) {
    nodes.clear();
    edges.clear();
    scene->clear();

    int leftX = 50;
    int rightX = 750;
    int centerY = 300;

    nodes.emplace_back(leftX, centerY);
    nodes.emplace_back(rightX, centerY);

    for (int i = 2; i < nodeCount; ++i) {
        int x = QRandomGenerator::global()->bounded(leftX + 1, rightX);
        int y = QRandomGenerator::global()->bounded(50, 550);
        nodes.emplace_back(x, y);
    }

    for (int i = 0; i < edgeCount; ++i) {
        int node1 = QRandomGenerator::global()->bounded(nodeCount);
        int node2 = QRandomGenerator::global()->bounded(nodeCount);

        if (node1 != node2 && !((node1 == 0 && node2 == 1) || (node1 == 1 && node2 == 0))) {
            double distance = std::hypot(nodes[node1].first - nodes[node2].first,
                                         nodes[node1].second - nodes[node2].second);
            edges.emplace_back(node1, node2, distance);
        } else {
            i--;
            continue;
        }
    }


    drawMap();
    isPressed = false;
}

void RoadMap::drawMap() {
    for (const auto &node : nodes) {
        scene->addEllipse(node.first - 3, node.second - 3, 6, 6,
                          QPen(Qt::black), QBrush(Qt::black));
    }

    for (const auto &edge : edges) {
        int node1 = std::get<0>(edge);
        int node2 = std::get<1>(edge);
        scene->addLine(nodes[node1].first, nodes[node1].second,
                       nodes[node2].first, nodes[node2].second,
                       QPen(Qt::blue));
    }
}

void RoadMap::findOptimalPath() {
    if(isPressed) return;

    int numNodes = nodes.size();
    if (numNodes == 0) return;

    std::vector<std::vector<double>> adjacencyMatrix(numNodes, std::vector<double>(numNodes, 0.0));
    for (const auto &edge : edges) {
        int node1 = std::get<0>(edge);
        int node2 = std::get<1>(edge);
        double distance = std::get<2>(edge);
        adjacencyMatrix[node1][node2] = distance;
        adjacencyMatrix[node2][node1] = distance;
    }

    std::vector<std::vector<double>> pheromones(numNodes, std::vector<double>(numNodes, 1.0));

    std::vector<int> bestPath;
    double bestPathLength = std::numeric_limits<double>::max();

    int startNode = 0;
    int endNode = 1;

    for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
        std::vector<std::vector<int>> antPaths(NUM_ANTS);
        std::vector<double> pathLengths(NUM_ANTS, 0.0);

        for (int ant = 0; ant < NUM_ANTS; ++ant) {
            std::vector<bool> visited(numNodes, false);
            int current = startNode;
            antPaths[ant].push_back(current);
            visited[current] = true;

            while (current != endNode) {
                std::vector<double> probabilities(numNodes, 0.0);
                double totalProbability = 0.0;

                for (int i = 0; i < numNodes; ++i) {
                    if (!visited[i] && adjacencyMatrix[current][i] > 0) {
                        double pheromone = pheromones[current][i];
                        double distance = adjacencyMatrix[current][i];
                        probabilities[i] = pow(pheromone, ALPHA) * pow(1.0 / distance, BETA);
                        totalProbability += probabilities[i];
                    }
                }

                if (totalProbability == 0.0) {
                    break;
                }

                double randomValue = QRandomGenerator::global()->generateDouble() * totalProbability;
                double cumulativeProbability = 0.0;
                int next = -1;

                for (int i = 0; i < numNodes; ++i) {
                    if (probabilities[i] > 0) {
                        cumulativeProbability += probabilities[i];
                        if (randomValue <= cumulativeProbability) {
                            next = i;
                            break;
                        }
                    }
                }

                if (next < 0 || next >= numNodes) {
                    break;
                }

                antPaths[ant].push_back(next);
                pathLengths[ant] += adjacencyMatrix[current][next];
                visited[next] = true;
                current = next;
            }
            if(adjacencyMatrix[current][endNode]!=0.0){
                pathLengths[ant] += adjacencyMatrix[current][endNode];
                antPaths[ant].push_back(endNode);
            }
        }

        for (int i = 0; i < numNodes; ++i) {
            for (int j = 0; j < numNodes; ++j) {
                pheromones[i][j] *= (1.0 - EVAPORATION); // Испарение феромонов
            }
        }

        for (int ant = 0; ant < NUM_ANTS; ++ant) {
            double pheromoneDeposit = Q / pathLengths[ant];
            for (size_t step = 0; step < antPaths[ant].size() - 1; ++step) {
                int from = antPaths[ant][step];
                int to = antPaths[ant][step + 1];
                pheromones[from][to] += pheromoneDeposit;
                pheromones[to][from] += pheromoneDeposit;
            }

            if (pathLengths[ant] < bestPathLength && (antPaths[ant][antPaths[ant].size()-1]==1)) {
                bestPathLength = pathLengths[ant];
                bestPath = antPaths[ant];
                // for (size_t i = 0; i < bestPath.size() - 1; ++i) {
                //     int from = bestPath[i];
                //     int to = bestPath[i + 1];
                //     qDebug()<<"====";
                //     qDebug()<<from;
                //     qDebug()<<to;
                //     qDebug()<<adjacencyMatrix[from][to];
                // }
            }
        }
        isPressed = true;
    }

    if(!bestPath.empty()) {
        for (size_t i = 0; i < bestPath.size() - 1; ++i) {
            int from = bestPath[i];
            int to = bestPath[i + 1];
            scene->addLine(nodes[from].first, nodes[from].second,
                           nodes[to].first, nodes[to].second,
                           QPen(Qt::red, 2));
        }
        drawBestPathLength(bestPathLength);
    }
    else {
        drawNoPath();
    }
}
void RoadMap::drawNoPath() {
    helpWindow = new QWidget(this);
    helpWindow->setWindowTitle("Печаль(");
    helpWindow->resize(200, 150);
    helpWindow->move(0, 400);

    QVBoxLayout *layout = new QVBoxLayout(helpWindow);

    QTextEdit *helpText = new QTextEdit(helpWindow);
    helpText->setReadOnly(true);
    helpText->setAlignment(Qt::AlignCenter);
    helpText->setText("Маршрута не существует\n");
    helpText->setStyleSheet("font: 14pt; color: eae0d5; background-color: #415A77; border: 1px solid #cccccc; border-radius: 5px;");
    layout->addWidget(helpText);

    QPushButton *closeButton = new QPushButton("Закрыть", helpWindow);
    closeButton->setStyleSheet("font: bold 12pt; background-color: #1b263b; color: eae0d5; border-radius: 5px;");
    connect(closeButton, &QPushButton::clicked, helpWindow, &QWidget::close);
    layout->addWidget(closeButton);

    helpWindow->setLayout(layout);
    helpWindow->show();
}
void RoadMap::drawBestPathLength(double len) {
    helpWindow = new QWidget(this);
    helpWindow->setWindowTitle("Лучший путь длиной");
    helpWindow->resize(200, 150);
    helpWindow->move(0, 400);

    QVBoxLayout *layout = new QVBoxLayout(helpWindow);

    QTextEdit *helpText = new QTextEdit(helpWindow);
    helpText->setReadOnly(true);
    helpText->setAlignment(Qt::AlignCenter);
    helpText->setText("Лучший путь длиной: \n" + QString::number(len, 'f', 2));
    helpText->setStyleSheet("font: 14pt; color: eae0d5; background-color: #415A77; border: 1px solid #cccccc; border-radius: 5px;");
    layout->addWidget(helpText);

    QPushButton *closeButton = new QPushButton("Закрыть", helpWindow);
    closeButton->setStyleSheet("font: bold 12pt; background-color: #1b263b; color: eae0d5; border-radius: 5px;");
    connect(closeButton, &QPushButton::clicked, helpWindow, &QWidget::close);
    layout->addWidget(closeButton);

    helpWindow->setLayout(layout);
    helpWindow->show();
}
