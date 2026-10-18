#pragma once
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

class Elastic {
public:
    explicit Elastic(std::string url);

    void put(const std::string& index, const nlohmann::json& doc);
    void remove(const std::string& index, int id);
    void reset(const std::string& index, const std::vector<nlohmann::json>& docs);
    std::set<int> search(const std::string& index, const std::string& name, const nlohmann::json& filters);

private:
    std::string url;

    nlohmann::json call(const std::string& method, const std::string& path, const std::string& body = "",
                        const std::string& type = "application/json");
};
