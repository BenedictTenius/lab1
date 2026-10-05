export module PipeModule;

export struct Pipe {
    double len;
    double diameter;
    const char* material;
    bool inRepair;

    std::iostream& operator<<(std::iostream& out, const Pipe& pipe) {
        out
            << ", Длина: " << pipe.len
            << ", Диаметр: " << pipe.diameter
            << ", Материал: " << pipe.material
            << ", В ремонте: " << (pipe.inRepair ? "Да" : "Нет");

        return out;
    }
};
