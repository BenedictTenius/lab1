export module PipeModule;

export struct Pipe {
    const char* name;
    double len;
    double diameter;
    const char* material;
    bool inRepair;

    std::iostream& operator<<(std::iostream& out, const Pipe& pipe) {
        out << "Труба: " << pipe.name
            << ", Длина: " << pipe.len
            << ", Диаметр: " << pipe.diameter
            << ", Материал: " << pipe.material
            << ", В ремонте: " << (pipe.inRepair ? "Да" : "Нет");

        return out;
    }
};
