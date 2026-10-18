#include "console.h"
#include "utils/input.h"

template <typename T>
static int inputId(const std::map<int, T>& items, const std::string& prompt)
{
    int id;
    while (!items.count(id = input<int>(prompt)))
        std::cout << "No object with this id\n";
    return id;
}

template <typename T>
static std::set<int> choose(const std::map<int, T>& items, const std::set<int>& found)
{
    for (int id : found)
        std::cout << items.at(id) << '\n';
    if (found.empty()) {
        std::cout << "Nothing found\n";
        return found;
    }
    if (input<int>("1 - all found, 2 - choose ids: ", 1, 2) == 1)
        return found;
    std::set<int> chosen;
    std::istringstream in(inputLine("Ids separated by spaces: "));
    for (int id; in >> id;)
        if (found.count(id))
            chosen.insert(id);
    std::cout << "Selected " << chosen.size() << " objects\n";
    return chosen;
}

Console::Console(Network& network) : network(network) {}

void Console::run()
{
    while (true) {
        std::cout << "\n1. Add pipe\n2. Add station\n3. Show all\n4. Search & edit pipes\n5. Search & edit stations\n"
                     "6. Save\n7. Load\n8. Connect stations\n9. Disconnect pipe\n10. Topological sort\n"
                     "11. Max flow\n12. Shortest path\n0. Exit\n";
        try {
            switch (input<int>("> ", 0, 12)) {
            case 1: addPipe(); break;
            case 2: addStation(); break;
            case 3: show(); break;
            case 4: editPipes(); break;
            case 5: editStations(); break;
            case 6: save(); break;
            case 7: load(); break;
            case 8: connect(); break;
            case 9: disconnect(); break;
            case 10: topologicalSort(); break;
            case 11: maxFlow(); break;
            case 12: shortestPath(); break;
            case 0: return;
            }
        } catch (const std::exception& error) {
            std::cout << "Error: " << error.what() << '\n';
        }
    }
}

void Console::addPipe()
{
    Pipe pipe;
    pipe.read();
    std::cout << "Added pipe #" << network.addPipe(pipe) << '\n';
}

void Console::addStation()
{
    Station station;
    station.read();
    std::cout << "Added station #" << network.addStation(station) << '\n';
}

void Console::show() const
{
    std::cout << "--- Pipes (" << network.getPipes().size() << ") ---\n";
    for (const auto& [id, pipe] : network.getPipes())
        std::cout << pipe << '\n';
    std::cout << "--- Stations (" << network.getStations().size() << ") ---\n";
    for (const auto& [id, station] : network.getStations())
        std::cout << station << '\n';
}

void Console::editPipes()
{
    std::string name = inputLine("Name contains (empty - any): ");
    int repair = input<int>("In repair: 0 - any, 1 - yes, 2 - no: ", 0, 2);
    std::set<int> ids = choose(network.getPipes(), network.findPipes(name, repair));
    if (ids.empty())
        return;
    int action = input<int>("1 - set in repair, 2 - set working, 3 - delete, 0 - cancel: ", 0, 3);
    for (int id : ids)
        if (action == 3)
            network.deletePipe(id);
        else if (action)
            network.setInRepair(id, action == 1);
}

void Console::editStations()
{
    std::string name = inputLine("Name contains (empty - any): ");
    double low = input<double>("Unused workshops % from: ", 0, 100);
    double high = input<double>("Unused workshops % to: ", low, 100);
    std::set<int> ids = choose(network.getStations(), network.findStations(name, low, high));
    if (ids.empty())
        return;
    int action = input<int>("1 - start workshop, 2 - stop workshop, 3 - delete, 0 - cancel: ", 0, 3);
    for (int id : ids)
        if (action == 3)
            network.deleteStation(id);
        else if (action && !network.changeWorkshops(id, action == 1))
            std::cout << "Station #" << id << ": no workshop to " << (action == 1 ? "start" : "stop") << '\n';
}

void Console::save()
{
    network.save(inputName("File name: "));
    std::cout << "Saved\n";
}

void Console::load()
{
    network.load(inputName("File name: "));
    std::cout << "Loaded " << network.getPipes().size() << " pipes and " << network.getStations().size()
              << " stations\n";
}

void Console::connect()
{
    auto [from, to] = inputRoute();
    int diameter = inputDiameter();
    int id = network.findFreePipe(diameter);
    if (!id) {
        std::cout << "No free pipe with this diameter, creating a new one\n";
        Pipe pipe;
        pipe.read(diameter);
        id = network.addPipe(pipe);
    }
    network.connect(id, from, to);
    std::cout << "Connected: " << network.getPipes().at(id) << '\n';
}

void Console::disconnect()
{
    if (network.getPipes().empty())
        throw std::runtime_error("No pipes");
    network.disconnect(inputId(network.getPipes(), "Pipe id: "));
}

void Console::topologicalSort() const
{
    std::vector<int> order;
    if (!network.graph().topologicalSort(order))
        throw std::runtime_error("Network has a cycle, topological sort is impossible");
    std::cout << "Topological order:";
    for (int station : order)
        std::cout << ' ' << station;
    std::cout << '\n';
}

void Console::maxFlow() const
{
    auto [source, sink] = inputRoute();
    std::cout << "Max flow from " << source << " to " << sink << ": " << network.graph().maxFlow(source, sink) << '\n';
}

void Console::shortestPath() const
{
    auto [from, to] = inputRoute();
    double length = 0;
    std::vector<int> path = network.graph().shortestPath(from, to, length);
    if (path.empty())
        throw std::runtime_error("No path from " + std::to_string(from) + " to " + std::to_string(to));
    std::cout << "Shortest path (" << length << " km):";
    for (int station : path)
        std::cout << ' ' << station;
    std::cout << '\n';
}

std::pair<int, int> Console::inputRoute() const
{
    if (network.getStations().size() < 2)
        throw std::runtime_error("Need at least 2 stations");
    int from = inputId(network.getStations(), "From station id: ");
    int to = inputId(network.getStations(), "To station id: ");
    network.checkRoute(from, to);
    return {from, to};
}
