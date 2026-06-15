#pragma once
#include <string>
#include <unordered_map>

class Config {
public:
    explicit Config(const std::string& path);

    const std::string& appId() const;
    const std::string& appSecret() const;
    const std::string& receiveIdType() const;

    // Resolve contact name to ID. Returns empty if not found.
    // id_type: the configured receive_id_type, used to decide whether
    //          non-open_id values can be passed through directly.
    std::string resolveContact(const std::string& name,
                               const std::string& id_type = "open_id") const;

    // Returns true if the contact name exists in config.
    bool hasContact(const std::string& name) const;

private:
    std::string app_id_;
    std::string app_secret_;
    std::string receive_id_type_;
    std::unordered_map<std::string, std::string> contacts_;
};
