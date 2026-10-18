#pragma once
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>

class Station {
public:
    int id = 0, workshops = 1, working = 0, stationClass = 1;
    std::string name;

    void read();
    bool start();
    bool stop();
    double unusedPercent() const;
    bool valid() const;
    nlohmann::json document() const;

    friend std::ostream& operator<<(std::ostream& out, const Station& station);
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Station, id, name, workshops, working, stationClass)
};
