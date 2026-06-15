#pragma once
#include <string>
#include <memory>

class Auth;

class FeishuClient {
public:
    FeishuClient(std::shared_ptr<Auth> auth);
    ~FeishuClient();

    // Send a text message
    void sendText(const std::string& receive_id, const std::string& text,
                  const std::string& receive_id_type = "open_id");

    // Send an image (local file path). Supports png, jpg, jpeg, gif, webp.
    void sendImage(const std::string& receive_id, const std::string& file_path,
                   const std::string& receive_id_type = "open_id");

    // Send a file (local file path).
    void sendFile(const std::string& receive_id, const std::string& file_path,
                  const std::string& receive_id_type = "open_id");

    // Send a video (local file path).
    void sendVideo(const std::string& receive_id, const std::string& file_path,
                   const std::string& receive_id_type = "open_id");

private:
    // Upload an image, returns image_key
    std::string uploadImage(const std::string& file_path);

    // Upload a file, returns file_key
    std::string uploadFile(const std::string& file_path);

    // Low-level send message
    void sendMessage(const std::string& receive_id,
                     const std::string& msg_type,
                     const std::string& content_json,
                     const std::string& receive_id_type);

    // Get file extension from path (without dot)
    static std::string fileExtension(const std::string& path);

    // Get file name from path
    static std::string fileName(const std::string& path);

    std::shared_ptr<Auth> auth_;
    std::string api_base_;
};
