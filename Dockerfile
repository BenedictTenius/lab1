FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends g++ pkg-config libcurl4-openssl-dev \
    nlohmann-json3-dev qt6-base-dev qt6-qpa-plugins fonts-dejavu-core && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY src src
RUN CORE="$(find src/models src/network src/services src/utils -name '*.cpp')" \
    && g++ -std=c++17 -O2 -Wall -Wextra -Isrc -o pipeline src/main.cpp src/ui/console.cpp $CORE -lcurl \
    && g++ -std=c++17 -O2 -Wall -Wextra -fPIC -Isrc -o pipeline-gui src/gui_main.cpp src/ui/window.cpp $CORE \
       $(pkg-config --cflags --libs Qt6Widgets) -lcurl
WORKDIR /app/data
CMD ["/app/pipeline"]
