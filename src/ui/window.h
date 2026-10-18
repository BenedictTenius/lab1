#pragma once
#include "network/network.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <functional>

class Window : public QMainWindow {
public:
    explicit Window(Network& network);

private:
    Network& network;
    QTableWidget* pipeTable = new QTableWidget(0, 7);
    QTableWidget* stationTable = new QTableWidget(0, 6);
    QLineEdit* pipeName = new QLineEdit;
    QComboBox* pipeRepair = new QComboBox;
    QLineEdit* stationName = new QLineEdit;
    QDoubleSpinBox* unusedFrom = new QDoubleSpinBox;
    QDoubleSpinBox* unusedTo = new QDoubleSpinBox;

    QWidget* pipesTab();
    QWidget* stationsTab();
    QPushButton* button(const QString& text, std::function<void()> action);
    void refresh();
    std::set<int> selected(QTableWidget* table) const;
    QComboBox* stationBox() const;
    bool askRoute(const QString& title, int& from, int& to);
    void addPipe(int diameter = 0, int from = 0, int to = 0);
    void addStation();
    void connectStations();
    void load();
    void save();
    void topologicalSort();
    void maxFlow();
    void shortestPath();
};
