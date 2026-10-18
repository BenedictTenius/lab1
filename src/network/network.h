#pragma once
#include "graph.h"
#include "models/pipe.h"
#include "models/station.h"
#include "services/elastic.h"
#include <map>
#include <set>

class Network {
public:
    explicit Network(Elastic elastic);

    const std::map<int, Pipe>& getPipes() const;
    const std::map<int, Station>& getStations() const;
    int addPipe(Pipe pipe);
    int addStation(Station station);
    void setInRepair(int id, bool inRepair);
    bool changeWorkshops(int id, bool start);
    void deletePipe(int id);
    void deleteStation(int id);
    std::set<int> findPipes(const std::string& name, int repair);
    std::set<int> findStations(const std::string& name, double low, double high);
    void save(const std::string& path) const;
    void load(const std::string& path);
    void checkRoute(int from, int to) const;
    int findFreePipe(int diameter) const;
    void connect(int pipeId, int from, int to);
    void disconnect(int pipeId);
    Graph graph() const;

private:
    std::map<int, Pipe> pipes;
    std::map<int, Station> stations;
    int nextPipeId = 1, nextStationId = 1;
    Elastic elastic;

    void sync(const Pipe& pipe);
    void sync(const Station& station);
    void reindex();
    Pipe& pipe(int id);
    Station& station(int id);
};
