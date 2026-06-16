#include "config.h"
#include "auth.h"
#include "feishu_client.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <windows.h>

static void printUsage() {
    std::cerr <<
        "flybook - Send messages to Feishu contacts\n\n"
        "Usage:\n"
        "  flybook.exe [--config <path>] send text  --to <contact> --text <message>\n"
        "  flybook.exe [--config <path>] send image --to <contact> --file <path>\n"
        "  flybook.exe [--config <path>] send file  --to <contact> --file <path>\n"
        "  flybook.exe [--config <path>] send video --to <contact> --file <path>\n\n"
        "Options:\n"
        "  --config <path>   Path to config.json (default: config.json next to exe)\n"
        "  --to <contact>    Contact name from config, or ID (open_id / user_id)\n"
        "  --text <message>  Text content to send\n"
        "  --file <path>     Path to the media file\n"
        "  --help            Show this help\n";
}

static std::string toUtf8(const std::string& input) {
    if (input.empty()) return input;
    int wlen = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, nullptr, 0);
    if (wlen == 0) return input;
    std::wstring wide(wlen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, wide.data(), wlen);
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ulen == 0) return input;
    std::string utf8(ulen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), ulen, nullptr, nullptr);
    if (!utf8.empty() && utf8.back() == '\0') utf8.pop_back();
    return utf8;
}

static std::string exeDir() {
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len == 0) return ".";
    std::string path(buf, len);
    auto sep = path.rfind('\\');
    if (sep == std::string::npos) return ".";
    return path.substr(0, sep);
}

static std::vector<std::string> toArgs(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    return args;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args = toArgs(argc, argv);

    if (args.size() < 2) {
        printUsage();
        return 1;
    }

    // Parse --config first (must appear before subcommand)
    std::string config_path;
    size_t pos = 1;

    while (pos < args.size()) {
        if (args[pos] == "--help" || args[pos] == "-h") {
            printUsage();
            return 0;
        }
        if (args[pos] == "--config" && pos + 1 < args.size()) {
            config_path = args[pos + 1];
            pos += 2;
            continue;
        }
        break;
    }

    if (pos >= args.size() || args[pos] != "send") {
        std::cerr << "Error: expected 'send' subcommand\n\n";
        printUsage();
        return 1;
    }
    pos++;

    if (pos >= args.size()) {
        std::cerr << "Error: expected message type (text/image/file/video)\n\n";
        printUsage();
        return 1;
    }

    std::string msg_type = args[pos];
    pos++;

    if (msg_type != "text" && msg_type != "image" &&
        msg_type != "file" && msg_type != "video") {
        std::cerr << "Error: unknown message type '" << msg_type
                  << "' (expected: text, image, file, video)\n";
        return 1;
    }

    std::string to;
    std::string text;
    std::string file;

    while (pos < args.size()) {
        if (args[pos] == "--to" && pos + 1 < args.size()) {
            to = args[pos + 1];
            pos += 2;
        } else if (args[pos] == "--text" && pos + 1 < args.size()) {
            text = args[pos + 1];
            pos += 2;
        } else if (args[pos] == "--file" && pos + 1 < args.size()) {
            file = args[pos + 1];
            pos += 2;
        } else if (args[pos] == "--config") {
            std::cerr << "Error: --config must come before 'send'\n";
            return 1;
        } else {
            std::cerr << "Error: unknown option: " << args[pos] << "\n";
            return 1;
        }
    }

    // Convert args from system code page to UTF-8
    to = toUtf8(to);
    text = toUtf8(text);
    file = toUtf8(file);

    // Validate required args
    if (to.empty()) {
        std::cerr << "Error: --to <contact> is required\n";
        return 1;
    }
    if (msg_type == "text" && text.empty()) {
        std::cerr << "Error: --text <message> is required for text messages\n";
        return 1;
    }
    if (msg_type != "text" && file.empty()) {
        std::cerr << "Error: --file <path> is required for " << msg_type << " messages\n";
        return 1;
    }

    // Resolve config path
    if (config_path.empty()) {
        config_path = exeDir() + "\\config.json";
    }

    // Execute
    try {
        Config config(config_path);
        std::string id_type = config.receiveIdType();
        std::string receive_id = config.resolveContact(to, id_type);

        if (receive_id.empty()) {
            std::cerr << "Error: unknown contact '" << to
                      << "'. Add it to config.json or use an ID directly.\n";
            return 1;
        }

        auto auth = std::make_shared<Auth>(
            config.appId(), config.appSecret(), exeDir());
        FeishuClient client(auth);

        if (msg_type == "text") {
            client.sendText(receive_id, text, id_type);
        } else if (msg_type == "image") {
            client.sendImage(receive_id, file, id_type);
        } else if (msg_type == "file") {
            client.sendFile(receive_id, file, id_type);
        } else if (msg_type == "video") {
            client.sendVideo(receive_id, file, id_type);
        }

        std::cout << "OK" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
