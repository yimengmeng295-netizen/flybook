#include "auth.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

Auth::Auth(const std::string& app_id, const std::string& app_secret,
           const std::string& cache_dir)
    : app_id_(app_id), app_secret_(app_secret) {
    cache_path_ = cache_dir + "/.token_cache";
}

std::string Auth::getToken() {
    if (loadFromCache()) {
        return token_;
    }
    token_ = fetchFromServer();
    saveToCache();
    return token_;
}

bool Auth::loadFromCache() {
    std::ifstream f(cache_path_);
    if (!f.is_open()) return false;

    json data;
    try {
        data = json::parse(f);
    } catch (...) {
        return false;
    }

    if (!data.contains("token") || !data.contains("expires_at")) return false;

    auto expires_at = std::chrono::system_clock::from_time_t(
        data["expires_at"].get<time_t>());
    if (std::chrono::system_clock::now() >= expires_at) return false;

    token_ = data["token"].get<std::string>();
    expires_at_ = expires_at;
    return true;
}

void Auth::saveToCache() {
    json data;
    data["token"] = token_;
    data["expires_at"] = std::chrono::system_clock::to_time_t(expires_at_);

    std::ofstream f(cache_path_);
    if (f.is_open()) {
        f << data.dump();
    }
}

std::string Auth::fetchFromServer() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to init curl");
    }

    std::string url = "https://open.feishu.cn/open-apis/auth/v3/tenant_access_token/internal";
    json req_body = {
        {"app_id", app_id_},
        {"app_secret", app_secret_}
    };
    std::string req_str = req_body.dump();
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("Token request failed: ") + curl_easy_strerror(res));
    }
    if (http_code != 200) {
        throw std::runtime_error("Token request HTTP " + std::to_string(http_code) + ": " + response);
    }

    json resp = json::parse(response);
    if (resp.value("code", -1) != 0) {
        throw std::runtime_error("Token API error: " + resp.value("msg", "unknown"));
    }

    std::string token = resp["tenant_access_token"].get<std::string>();
    int64_t expire = resp.value("expire", 7200);

    // Set expiry with 60s buffer
    expires_at_ = std::chrono::system_clock::now() + std::chrono::seconds(expire - 60);

    return token;
}
