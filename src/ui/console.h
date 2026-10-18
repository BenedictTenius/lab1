#pragma once
#include "network/network.h"

class Console {
public:
    explicit Console(Network& network);
    void run();

private:
    Network& network;

    void addPipe();
    void addStation();
    void show() const;
    void editPipes();
    void editStations();
    void save();
    void load();
    void connect();
    void disconnect();
    void topologicalSort() const;
    void maxFlow() const;
    void shortestPath() const;
    std::pair<int, int> inputRoute() const;
};
