#include "ui/console.h"
#include <curl/curl.h>

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    const char* url = std::getenv("ELASTIC_URL");
    Network network(Elastic(url ? url : "http://localhost:9200"));
    Console(network).run();
    curl_global_cleanup();
}
