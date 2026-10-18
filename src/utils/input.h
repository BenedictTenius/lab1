#pragma once
#include <array>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

const std::array<int, 4> DIAMETERS = {500, 700, 1000, 1400};

void logAction(const std::string& text);
std::string inputLine(const std::string& prompt);
std::string inputName(const std::string& prompt);
int inputDiameter();
bool isAllowedDiameter(int diameter);

template <typename T>
T input(const std::string& prompt, T min = std::numeric_limits<T>::lowest(), T max = std::numeric_limits<T>::max())
{
    while (true) {
        std::istringstream in(inputLine(prompt));
        T value;
        if (in >> value && (in >> std::ws).eof() && value >= min && value <= max)
            return value;
        std::cout << "Enter a number in range [" << min << ", " << max << "]\n";
    }
}
