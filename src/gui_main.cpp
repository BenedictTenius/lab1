#include "ui/window.h"
#include <QApplication>
#include <curl/curl.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    const char* url = std::getenv("ELASTIC_URL");
    Network network(Elastic(url ? url : "http://localhost:9200"));
    Window window(network);
    window.show();
    int code = app.exec();
    curl_global_cleanup();
    return code;
}
