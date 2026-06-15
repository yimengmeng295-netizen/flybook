#include "feishu_client.h"
#include "auth.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fstream>
#include <filesystem>
#include <unordered_map>

using json = nlohmann::json;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

FeishuClient::FeishuClient(std::shared_ptr<Auth> auth)
    : auth_(std::move(auth))
    , api_base_("https://open.feishu.cn/open-apis") {}

FeishuClient::~FeishuClient() = default;

// ---- Public API ----

void FeishuClient::sendText(const std::string& receive_id, const std::string& text,
                             const std::string& receive_id_type) {
    json content = {{"text", text}};
    sendMessage(receive_id, "text", content.dump(), receive_id_type);
}

void FeishuClient::sendImage(const std::string& receive_id, const std::string& file_path,
                              const std::string& receive_id_type) {
    std::string image_key = uploadImage(file_path);
    json content = {{"image_key", image_key}};
    sendMessage(receive_id, "image", content.dump(), receive_id_type);
}

void FeishuClient::sendFile(const std::string& receive_id, const std::string& file_path,
                             const std::string& receive_id_type) {
    std::string file_key = uploadFile(file_path);
    json content = {{"file_key", file_key}};
    sendMessage(receive_id, "file", content.dump(), receive_id_type);
}

void FeishuClient::sendVideo(const std::string& receive_id, const std::string& file_path,
                              const std::string& receive_id_type) {
    std::string file_key = uploadFile(file_path);
    json content = {{"file_key", file_key}};
    sendMessage(receive_id, "media", content.dump(), receive_id_type);
}

// ---- Upload helpers ----

std::string FeishuClient::uploadImage(const std::string& file_path) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string url = api_base_ + "/im/v1/images";
    std::string token = auth_->getToken();
    std::string response;

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part;

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "image_type");
    curl_mime_data(part, "message", CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "image");
    curl_mime_filedata(part, file_path.c_str());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers,
        ("Authorization: Bearer " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    curl_mime_free(mime);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("Image upload failed: ") + curl_easy_strerror(res));
    }
    if (http_code != 200) {
        throw std::runtime_error("Image upload HTTP " + std::to_string(http_code) + ": " + response);
    }

    json resp = json::parse(response);
    if (resp.value("code", -1) != 0) {
        throw std::runtime_error("Image upload API error: " + resp.value("msg", "unknown"));
    }

    return resp["data"]["image_key"].get<std::string>();
}

std::string FeishuClient::uploadFile(const std::string& file_path) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string url = api_base_ + "/im/v1/files";
    std::string token = auth_->getToken();
    std::string response;

    std::string fname = fileName(file_path);
    std::string fext = fileExtension(file_path);

    // Map extension to Feishu file_type (only opus/mp4/pdf/doc/xls/ppt/stream are valid)
    static const std::unordered_map<std::string, std::string> fileTypeMap = {
        {"opus", "opus"}, {"mp4", "mp4"}, {"pdf", "pdf"},
        {"doc", "doc"}, {"docx", "doc"}, {"xls", "xls"}, {"xlsx", "xls"},
        {"ppt", "ppt"}, {"pptx", "ppt"}
    };
    auto ft = fileTypeMap.find(fext);
    std::string file_type = (ft != fileTypeMap.end()) ? ft->second : "stream";

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part;

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file_type");
    curl_mime_data(part, file_type.c_str(), CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file_name");
    curl_mime_data(part, fname.c_str(), CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, file_path.c_str());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers,
        ("Authorization: Bearer " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    curl_mime_free(mime);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("File upload failed: ") + curl_easy_strerror(res));
    }
    if (http_code != 200) {
        throw std::runtime_error("File upload HTTP " + std::to_string(http_code) + ": " + response);
    }

    json resp = json::parse(response);
    if (resp.value("code", -1) != 0) {
        throw std::runtime_error("File upload API error: " + resp.value("msg", "unknown"));
    }

    return resp["data"]["file_key"].get<std::string>();
}

// ---- Low-level send ----

void FeishuClient::sendMessage(const std::string& receive_id,
                                const std::string& msg_type,
                                const std::string& content_json,
                                const std::string& receive_id_type) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string url = api_base_
        + "/im/v1/messages?receive_id_type=" + receive_id_type;
    std::string token = auth_->getToken();
    std::string response;

    json body = {
        {"receive_id", receive_id},
        {"msg_type", msg_type},
        {"content", content_json}
    };
    std::string body_str = body.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers,
        ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers,
        "Content-Type: application/json; charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_str.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body_str.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("Send message failed: ") + curl_easy_strerror(res));
    }
    if (http_code != 200) {
        throw std::runtime_error("Send message HTTP " + std::to_string(http_code) + ": " + response);
    }

    json resp = json::parse(response);
    if (resp.value("code", -1) != 0) {
        throw std::runtime_error("Send message API error: " + resp.value("msg", "unknown"));
    }
}

// ---- Utility ----

std::string FeishuClient::fileExtension(const std::string& path) {
    auto dot = path.rfind('.');
    if (dot == std::string::npos) return {};
    auto ext = path.substr(dot + 1);
    // Normalize jpeg -> jpg for consistency
    if (ext == "jpeg") ext = "jpg";
    return ext;
}

std::string FeishuClient::fileName(const std::string& path) {
    auto sep = path.rfind('/');
    if (sep == std::string::npos) sep = path.rfind('\\');
    if (sep == std::string::npos) return path;
    return path.substr(sep + 1);
}
