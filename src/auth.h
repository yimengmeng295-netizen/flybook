#pragma once
#include <string>
#include <chrono>

class Auth {
public:
    // cache_dir: directory to store .token_cache file
    Auth(const std::string& app_id, const std::string& app_secret,
         const std::string& cache_dir);

    // Returns a valid tenant_access_token. Auto-refreshes if expired.
    std::string getToken();

private:
    bool loadFromCache();
    void saveToCache();
    std::string fetchFromServer();

    std::string app_id_;
    std::string app_secret_;
    std::string cache_path_;
    std::string token_;
    std::chrono::system_clock::time_point expires_at_{};
};
