#include "http.h"
#include <curl/curl.h>
#include <stdexcept>

nlohmann::json Http::send(const std::string& method, const std::string& url, const std::string& body,
                          const std::string& type)
{
    std::string answer;
    CURL* curl = curl_easy_init();
    curl_slist* headers = curl_slist_append(nullptr, ("Content-Type: " + type).c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    if (!body.empty())
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &answer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char* data, size_t size, size_t count, void* out) {
        static_cast<std::string*>(out)->append(data, size * count);
        return size * count;
    });
    CURLcode result = curl_easy_perform(curl);
    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (result != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(result));
    if (code >= 400 && code != 404)
        throw std::runtime_error("HTTP " + std::to_string(code) + " " + answer.substr(0, 300));
    return answer.empty() ? nlohmann::json() : nlohmann::json::parse(answer);
}
