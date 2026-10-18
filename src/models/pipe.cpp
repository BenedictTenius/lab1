#include "pipe.h"
#include "utils/input.h"
#include <cmath>

void Pipe::read(int fixedDiameter)
{
    name = inputName("Pipe name (km mark): ");
    length = input<double>("Length, km: ", 0.001, 100000);
    diameter = fixedDiameter ? fixedDiameter : inputDiameter();
    inRepair = input<int>("In repair (0/1): ", 0, 1);
}

bool Pipe::isFree() const
{
    return from == 0;
}

int Pipe::capacity() const
{
    return inRepair ? 0 : int(std::round(1000 * std::sqrt(std::pow(diameter / 1000.0, 5) / length)));
}

double Pipe::weight() const
{
    return inRepair ? std::numeric_limits<double>::infinity() : length;
}

bool Pipe::valid() const
{
    return !name.empty() && length > 0 && (from == 0) == (to == 0) && isAllowedDiameter(diameter);
}

nlohmann::json Pipe::document() const
{
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Pipe& pipe)
{
    out << "Pipe #" << pipe.id << " '" << pipe.name << "': " << pipe.length << " km, " << pipe.diameter << " mm, "
        << (pipe.inRepair ? "in repair" : "working");
    if (!pipe.isFree())
        out << ", connects station " << pipe.from << " -> " << pipe.to;
    return out;
}
