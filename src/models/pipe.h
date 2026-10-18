#pragma once
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>

class Pipe {
public:
    int id = 0, diameter = 500, from = 0, to = 0;
    std::string name;
    double length = 1;
    bool inRepair = false;

    void read(int fixedDiameter = 0);
    bool isFree() const;
    int capacity() const;
    double weight() const;
    bool valid() const;
    nlohmann::json document() const;

    friend std::ostream& operator<<(std::ostream& out, const Pipe& pipe);
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Pipe, id, name, length, diameter, inRepair, from, to)
};
