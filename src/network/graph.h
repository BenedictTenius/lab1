#pragma once
#include "models/pipe.h"
#include <map>
#include <vector>

class Graph {
public:
    explicit Graph(const std::map<int, Pipe>& pipes);

    bool topologicalSort(std::vector<int>& order) const;
    long long maxFlow(int source, int sink) const;
    std::vector<int> shortestPath(int from, int to, double& length) const;

private:
    std::vector<Pipe> edges;
};
