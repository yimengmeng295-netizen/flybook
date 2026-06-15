---
name: flybook
description: Send text, images, files, and videos to Feishu contacts via flybook.exe
user-invocable: true
metadata.openclaw: '{"os": ["win32"], "requires": {"bins": ["flybook"]}}'
---

# Flybook - 飞书消息发送工具

Use `flybook.exe` to send messages to Feishu contacts.

## Available commands

```
flybook.exe [--config <path>] send text  --to <contact> --text <message>
flybook.exe [--config <path>] send image --to <contact> --file <path>
flybook.exe [--config <path>] send file  --to <contact> --file <path>
flybook.exe [--config <path>] send video --to <contact> --file <path>
```

- `--config <path>`: Path to config.json. Default: config.json next to flybook.exe.
- `--to <contact>`: Contact name (as defined in config.json) or open_id starting with `ou_`.
- `--text <message>`: The text to send (text messages only).
- `--file <path>`: Path to the local file (image/file/video messages).

## Message type mapping

| User says | Use `send` type |
|-----------|----------------|
| "send a message" / "notify" / plain text | `text` |
| "send a photo" / "send a picture" / "send this image" | `image` |
| "send a document" / "send this file" / "attach" | `file` |
| "send a video" / "send this clip" | `video` |

## Important notes

- The executable is typically at `D:\flybook\build\flybook.exe`.
- Never guess file paths — confirm the file exists before invoking.
- If the contact name is not in config.json, ask the user for their open_id.
- Quoted text containing special characters should be wrapped properly for the shell.
