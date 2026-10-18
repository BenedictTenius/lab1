#include "graph.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <queue>

Graph::Graph(const std::map<int, Pipe>& pipes)
{
    for (const auto& [id, pipe] : pipes)
        if (!pipe.isFree())
            edges.push_back(pipe);
}

bool Graph::topologicalSort(std::vector<int>& order) const
{
    std::map<int, int> inDegree;
    std::map<int, std::vector<int>> next;
    for (const Pipe& pipe : edges) {
        next[pipe.from].push_back(pipe.to);
        inDegree[pipe.from];
        ++inDegree[pipe.to];
    }
    std::queue<int> ready;
    for (const auto& [id, degree] : inDegree)
        if (!degree)
            ready.push(id);
    order.clear();
    while (!ready.empty()) {
        int station = ready.front();
        ready.pop();
        order.push_back(station);
        for (int child : next[station])
            if (!--inDegree[child])
                ready.push(child);
    }
    return order.size() == inDegree.size();
}

long long Graph::maxFlow(int source, int sink) const
{
    std::map<int, std::map<int, long long>> capacity;
    for (const Pipe& pipe : edges) {
        capacity[pipe.from][pipe.to] += pipe.capacity();
        capacity[pipe.to][pipe.from] += 0;
    }
    long long flow = 0;
    while (true) {
        std::map<int, int> parent = {{source, source}};
        std::queue<int> queue;
        queue.push(source);
        while (!queue.empty() && !parent.count(sink)) {
            int station = queue.front();
            queue.pop();
            for (const auto& [next, rest] : capacity[station])
                if (rest > 0 && !parent.count(next)) {
                    parent[next] = station;
                    queue.push(next);
                }
        }
        if (!parent.count(sink))
            return flow;
        long long push = LLONG_MAX;
        for (int v = sink; v != source; v = parent[v])
            push = std::min(push, capacity[parent[v]][v]);
        for (int v = sink; v != source; v = parent[v]) {
            capacity[parent[v]][v] -= push;
            capacity[v][parent[v]] += push;
        }
        flow += push;
    }
}

std::vector<int> Graph::shortestPath(int from, int to, double& length) const
{
    std::map<int, std::vector<std::pair<int, double>>> next;
    for (const Pipe& pipe : edges)
        if (!std::isinf(pipe.weight()))
            next[pipe.from].push_back({pipe.to, pipe.weight()});
    std::map<int, double> distance = {{from, 0}};
    std::map<int, int> parent;
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> queue;
    queue.push({0, from});
    while (!queue.empty()) {
        auto [current, station] = queue.top();
        queue.pop();
        if (current > distance[station])
            continue;
        for (const auto& [child, weight] : next[station])
            if (!distance.count(child) || current + weight < distance[child]) {
                distance[child] = current + weight;
                parent[child] = station;
                queue.push({distance[child], child});
            }
    }
    if (!distance.count(to))
        return {};
    length = distance[to];
    std::vector<int> path = {to};
    while (path.back() != from)
        path.push_back(parent[path.back()]);
    std::reverse(path.begin(), path.end());
    return path;
}
