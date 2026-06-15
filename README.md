# Flybook

发送消息到飞书联系人的命令行工具，支持文本、图片、文件和视频。

## 快速开始

```bash
# 发送文本
flybook.exe send text  --to 张三 --text "你好"

# 发送图片
flybook.exe send image --to 张三 --file photo.png

# 发送文件
flybook.exe send file  --to 张三 --file doc.pdf

# 发送视频
flybook.exe send video --to 张三 --file video.mp4
```

## 配置

复制 `config.example.json` 为 `config.json`，填入你的飞书应用凭证。

| 配置项 | 说明 |
|--------|------|
| `app_id` | 飞书应用的 App ID |
| `app_secret` | 飞书应用的 App Secret |
| `receive_id_type` | ID 类型，`user_id` 或 `open_id` |
| `contacts` | 联系人名字到 ID 的映射 |

## 构建

```bash
# 依赖：CMake 3.16+, C++20 编译器, libcurl, vcpkg

vcpkg install curl:x64-windows

mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## 飞书应用权限

| 权限 | 用途 |
|------|------|
| `im:message` | 发送消息 |
| `im:image` | 上传和发送图片 |
| `im:file` | 上传和发送文件/视频 |
| `contact:user.employee_id:readonly` | 使用 user_id 时需要 |

## 开源协议

MIT License
