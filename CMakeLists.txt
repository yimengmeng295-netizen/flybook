cmake_minimum_required(VERSION 3.16)
project(flybook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Dependencies ----

# libcurl (install via vcpkg: vcpkg install curl:x64-windows)
find_package(CURL REQUIRED)

# nlohmann/json (header-only, fetched automatically)
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
)
FetchContent_MakeAvailable(json)

# ---- Source files ----

set(SOURCES
    src/main.cpp
    src/config.cpp
    src/auth.cpp
    src/feishu_client.cpp
)

set(HEADERS
    src/config.h
    src/auth.h
    src/feishu_client.h
)

# ---- Executable ----

add_executable(flybook ${SOURCES} ${HEADERS})

target_include_directories(flybook PRIVATE src)
target_link_libraries(flybook PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)

# ---- Install ----

install(TARGETS flybook RUNTIME DESTINATION .)
