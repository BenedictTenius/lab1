#include <iostream>

import PipeModule;

int main() {
    std::cout << "Input Pipe, https://github.com/BenedictTenius/lab1!\n";

    Pipe pipe{"Joba", 120 * 10 * 4, 12, "iron", false};
    std::cout << "Name: " << pipe.name
              << ", length: " << pipe.len
              << ", diameter: " << pipe.diameter
              << ", material: " << pipe.material
              << ", in repair: " << pipe.inRepair
              << '\n';

    return 0;
}
