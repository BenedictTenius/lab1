#pragma once
#include <nlohmann/json.hpp>
#include <string>

class Http {
public:
    static nlohmann::json send(const std::string& method, const std::string& url, const std::string& body = "",
                               const std::string& type = "application/json");
};
