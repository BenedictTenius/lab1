#include "elastic.h"
#include "http.h"
#include <iostream>

using json = nlohmann::json;

Elastic::Elastic(std::string url) : url(std::move(url)) {}

void Elastic::put(const std::string& index, const json& doc)
{
    call("PUT", "/" + index + "/_doc/" + std::to_string(doc["id"].get<int>()) + "?refresh=true", doc.dump());
}

void Elastic::remove(const std::string& index, int id)
{
    call("DELETE", "/" + index + "/_doc/" + std::to_string(id) + "?refresh=true");
}

void Elastic::reset(const std::string& index, const std::vector<json>& docs)
{
    call("DELETE", "/" + index + "?ignore_unavailable=true");
    call("PUT", "/" + index);
    std::string bulk;
    for (const json& doc : docs)
        bulk += json{{"index", {{"_id", std::to_string(doc["id"].get<int>())}}}}.dump() + '\n' + doc.dump() + '\n';
    if (!bulk.empty())
        call("POST", "/" + index + "/_bulk?refresh=true", bulk, "application/x-ndjson");
}

std::set<int> Elastic::search(const std::string& index, const std::string& name, const json& filters)
{
    json must = json::array();
    if (!name.empty())
        must.push_back({{"bool", {{"should", {
            {{"wildcard", {{"name.keyword", {{"value", "*" + name + "*"}, {"case_insensitive", true}}}}}},
            {{"match", {{"name", {{"query", name}, {"fuzziness", "AUTO"}, {"operator", "and"}}}}}},
        }}}}});
    json query = {{"size", 10000}, {"query", {{"bool", {{"must", must}, {"filter", filters}}}}}};
    json result = call("POST", "/" + index + "/_search", query.dump());
    std::set<int> ids;
    if (result.contains("hits"))
        for (const json& hit : result["hits"]["hits"])
            ids.insert(hit["_source"]["id"].get<int>());
    return ids;
}

json Elastic::call(const std::string& method, const std::string& path, const std::string& body, const std::string& type)
{
    try {
        return Http::send(method, url + path, body, type);
    } catch (const std::exception& error) {
        std::cerr << "Elasticsearch error: " << error.what() << '\n';
        return {};
    }
}
