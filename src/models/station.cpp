#include "station.h"
#include "utils/input.h"

void Station::read()
{
    name = inputName("Station name: ");
    workshops = input<int>("Workshops: ", 1, 1000);
    working = input<int>("Working workshops: ", 0, workshops);
    stationClass = input<int>("Station class (1-5): ", 1, 5);
}

bool Station::start()
{
    if (working == workshops)
        return false;
    ++working;
    return true;
}

bool Station::stop()
{
    if (working == 0)
        return false;
    --working;
    return true;
}

double Station::unusedPercent() const
{
    return 100.0 * (workshops - working) / workshops;
}

bool Station::valid() const
{
    return !name.empty() && workshops >= 1 && working >= 0 && working <= workshops;
}

nlohmann::json Station::document() const
{
    nlohmann::json doc = *this;
    doc["unused"] = unusedPercent();
    return doc;
}

std::ostream& operator<<(std::ostream& out, const Station& station)
{
    return out << "Station #" << station.id << " '" << station.name << "': workshops " << station.working << "/"
               << station.workshops << " (unused " << station.unusedPercent() << "%), class " << station.stationClass;
}
