#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Config::Config(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json data = json::parse(f);

    if (!data.contains("app_id") || !data["app_id"].is_string()) {
        throw std::runtime_error("config.json: missing or invalid 'app_id'");
    }
    app_id_ = data["app_id"].get<std::string>();

    if (!data.contains("app_secret") || !data["app_secret"].is_string()) {
        throw std::runtime_error("config.json: missing or invalid 'app_secret'");
    }
    app_secret_ = data["app_secret"].get<std::string>();

    receive_id_type_ = data.value("receive_id_type", "open_id");

    if (data.contains("contacts") && data["contacts"].is_object()) {
        for (auto& [name, id] : data["contacts"].items()) {
            if (id.is_string()) {
                contacts_[name] = id.get<std::string>();
            }
        }
    }
}

const std::string& Config::appId() const {
    return app_id_;
}

const std::string& Config::appSecret() const {
    return app_secret_;
}

const std::string& Config::receiveIdType() const {
    return receive_id_type_;
}

std::string Config::resolveContact(const std::string& name,
                                    const std::string& id_type) const {
    auto it = contacts_.find(name);
    if (it != contacts_.end()) {
        return it->second;
    }
    // If name looks like an open_id already (starts with "ou_"), use it directly
    if (name.starts_with("ou_")) {
        return name;
    }
    // If using user_id, allow any non-empty value to pass through directly
    if (id_type == "user_id" && !name.empty()) {
        return name;
    }
    return {};
}

bool Config::hasContact(const std::string& name) const {
    return contacts_.contains(name);
}
