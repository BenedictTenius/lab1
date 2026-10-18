#include "network.h"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

Network::Network(Elastic elastic) : elastic(std::move(elastic))
{
    reindex();
}

const std::map<int, Pipe>& Network::getPipes() const
{
    return pipes;
}

const std::map<int, Station>& Network::getStations() const
{
    return stations;
}

int Network::addPipe(Pipe pipe)
{
    if (!pipe.valid() || !pipe.isFree())
        throw std::runtime_error("Invalid pipe");
    pipe.id = nextPipeId++;
    sync(pipes[pipe.id] = pipe);
    return pipe.id;
}

int Network::addStation(Station station)
{
    if (!station.valid())
        throw std::runtime_error("Invalid station");
    station.id = nextStationId++;
    sync(stations[station.id] = station);
    return station.id;
}

void Network::setInRepair(int id, bool inRepair)
{
    pipe(id).inRepair = inRepair;
    sync(pipe(id));
}

bool Network::changeWorkshops(int id, bool start)
{
    if (!(start ? station(id).start() : station(id).stop()))
        return false;
    sync(station(id));
    return true;
}

void Network::deletePipe(int id)
{
    pipe(id);
    pipes.erase(id);
    elastic.remove("pipes", id);
}

void Network::deleteStation(int id)
{
    station(id);
    for (auto& [pipeId, item] : pipes)
        if (item.from == id || item.to == id) {
            item.from = item.to = 0;
            sync(item);
        }
    stations.erase(id);
    elastic.remove("stations", id);
}

std::set<int> Network::findPipes(const std::string& name, int repair)
{
    std::set<int> ids;
    if (name.empty() && !repair) {
        for (const auto& [id, item] : pipes)
            ids.insert(id);
        return ids;
    }
    json filters = json::array();
    if (repair)
        filters.push_back({{"term", {{"inRepair", repair == 1}}}});
    for (int id : elastic.search("pipes", name, filters))
        if (pipes.count(id))
            ids.insert(id);
    return ids;
}

std::set<int> Network::findStations(const std::string& name, double low, double high)
{
    std::set<int> ids;
    if (name.empty() && low <= 0 && high >= 100) {
        for (const auto& [id, item] : stations)
            ids.insert(id);
        return ids;
    }
    json filters = json::array({json{{"range", {{"unused", {{"gte", low}, {"lte", high}}}}}}});
    for (int id : elastic.search("stations", name, filters))
        if (stations.count(id))
            ids.insert(id);
    return ids;
}

void Network::save(const std::string& path) const
{
    std::ofstream out(path);
    if (!(out << json{{"pipes", pipes}, {"stations", stations}}.dump(2)))
        throw std::runtime_error("Can't write file " + path);
}

void Network::load(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("Can't open file " + path);
    json data = json::parse(in);
    auto newPipes = data.at("pipes").get<std::map<int, Pipe>>();
    auto newStations = data.at("stations").get<std::map<int, Station>>();
    for (const auto& [id, item] : newPipes)
        if (id != item.id || !item.valid() || (!item.isFree() && (item.from == item.to
            || !newStations.count(item.from) || !newStations.count(item.to))))
            throw std::runtime_error("Invalid pipe #" + std::to_string(id));
    for (const auto& [id, item] : newStations)
        if (id != item.id || !item.valid())
            throw std::runtime_error("Invalid station #" + std::to_string(id));
    pipes = std::move(newPipes);
    stations = std::move(newStations);
    nextPipeId = pipes.empty() ? 1 : pipes.rbegin()->first + 1;
    nextStationId = stations.empty() ? 1 : stations.rbegin()->first + 1;
    reindex();
}

void Network::checkRoute(int from, int to) const
{
    if (!stations.count(from) || !stations.count(to))
        throw std::runtime_error("No station with this id");
    if (from == to)
        throw std::runtime_error("Stations must be different");
}

int Network::findFreePipe(int diameter) const
{
    for (const auto& [id, item] : pipes)
        if (item.isFree() && item.diameter == diameter)
            return id;
    return 0;
}

void Network::connect(int pipeId, int from, int to)
{
    checkRoute(from, to);
    if (!pipe(pipeId).isFree())
        throw std::runtime_error("Pipe is already connected");
    pipe(pipeId).from = from;
    pipe(pipeId).to = to;
    sync(pipe(pipeId));
}

void Network::disconnect(int pipeId)
{
    pipe(pipeId).from = pipe(pipeId).to = 0;
    sync(pipe(pipeId));
}

Graph Network::graph() const
{
    return Graph(pipes);
}

void Network::sync(const Pipe& item)
{
    elastic.put("pipes", item.document());
}

void Network::sync(const Station& item)
{
    elastic.put("stations", item.document());
}

void Network::reindex()
{
    std::vector<json> pipeDocs, stationDocs;
    for (const auto& [id, item] : pipes)
        pipeDocs.push_back(item.document());
    for (const auto& [id, item] : stations)
        stationDocs.push_back(item.document());
    elastic.reset("pipes", pipeDocs);
    elastic.reset("stations", stationDocs);
}

Pipe& Network::pipe(int id)
{
    if (!pipes.count(id))
        throw std::runtime_error("No pipe #" + std::to_string(id));
    return pipes.at(id);
}

Station& Network::station(int id)
{
    if (!stations.count(id))
        throw std::runtime_error("No station #" + std::to_string(id));
    return stations.at(id);
}
