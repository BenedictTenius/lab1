#include "input.h"
#include <algorithm>
#include <fstream>

void logAction(const std::string& text)
{
    static std::ofstream log("log.txt", std::ios::app);
    log << text << std::endl;
}

std::string inputLine(const std::string& prompt)
{
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "\nInput closed, exit\n";
        std::exit(0);
    }
    logAction(prompt + line);
    return line;
}

std::string inputName(const std::string& prompt)
{
    std::string name;
    while ((name = inputLine(prompt)).empty())
        std::cout << "Name can't be empty\n";
    return name;
}

bool isAllowedDiameter(int diameter)
{
    return std::find(DIAMETERS.begin(), DIAMETERS.end(), diameter) != DIAMETERS.end();
}

int inputDiameter()
{
    while (true) {
        int diameter = input<int>("Diameter (500/700/1000/1400 mm): ");
        if (isAllowedDiameter(diameter))
            return diameter;
        std::cout << "Allowed diameters: 500, 700, 1000, 1400\n";
    }
}
