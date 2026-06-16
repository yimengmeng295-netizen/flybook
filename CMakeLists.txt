cmake_minimum_required(VERSION 3.16)
project(flybook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Dependencies (vendored, no network required) ----

# libcurl
set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/curl/include")
set(CURL_LIBRARY "${CMAKE_SOURCE_DIR}/vendor/curl/lib/libcurl.dll.a")
add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES
    IMPORTED_LOCATION "${CURL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}"
)

# nlohmann/json (header-only)
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE "${CMAKE_SOURCE_DIR}/vendor/json/include")

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
target_link_libraries(flybook PRIVATE curl nlohmann_json)

# ---- Install ----

install(TARGETS flybook RUNTIME DESTINATION .)
