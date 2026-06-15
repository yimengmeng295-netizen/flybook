---
name: flybook
description: Send text, images, files, and videos to Feishu contacts via flybook.exe
user-invocable: true
metadata.openclaw: '{"os": ["win32"], "requires": {"bins": ["flybook"]}}'
---

# Flybook - 飞书消息发送工具

Send messages to Feishu contacts via `flybook.exe`. Supports text, images, files, and videos with auto-detection of message type based on file extension.

## Available commands

```
flybook.exe [--config <path>] send text  --to <contact> --text <message>
flybook.exe [--config <path>] send image --to <contact> --file <path>
flybook.exe [--config <path>] send file  --to <contact> --file <path>
flybook.exe [--config <path>] send video --to <contact> --file <path>
```

- `--config <path>`: Path to config.json. Default: config.json next to flybook.exe (`D:\npm-global\flybook\config.json`).
- `--to <contact>`: Contact name (as defined in config.json), user_id, or open_id starting with `ou_`.
- `--text <message>`: The text to send (text messages only).
- `--file <path>`: Path to the local file (image/file/video messages).

## Auto-detection rules

When the user says "给飞书发xxx" or "send to Feishu xxx", determine the message type:

1. **If the user specifies a file path** (e.g., "发送这张图片给飞书", "把这个视频发过去"):
   - `.png` `.jpg` `.jpeg` `.gif` `.webp` → `send image`
   - `.mp4` `.mov` `.avi` `.mkv` `.webm` → `send video`
   - Everything else (`.pdf` `.doc` `.txt` `.zip` etc.) → `send file`
   - Always verify the file exists on disk before invoking.

2. **If the user sends plain text** (e.g., "发消息：会议开始了", "notify: build passed"):
   - Use `send text`

3. **If the user mentions an image/photo/picture without a file path**:
   - Ask the user to provide the file path.

## Message type mapping

| User says | Use `send` type |
|-----------|----------------|
| "send a message" / "notify" / plain text | `text` |
| "send a photo" / "send a picture" / "send this image" | `image` |
| "send a document" / "send this file" / "attach" | `file` |
| "send a video" / "send this clip" | `video` |

## Config

```json
{
    "app_id": "cli_xxxxxxxx",
    "app_secret": "your_app_secret_here",
    "receive_id_type": "user_id",
    "contacts": {
        "联系人名": "用户ID"
    }
}
```

Config file location: `D:\npm-global\flybook\config.json`.

- Contact names are resolved from the `contacts` map
- Can also use user_id (alphanumeric) or open_id (starts with `ou_`) directly
- See `config.example.json` in the repo for a template

## Installation for OpenClaw

### 1. Download the portable package

Download `flybook-portable.zip` from [GitHub Releases](https://github.com/yimengmeng295-netizen/flybook/releases).

### 2. Install to OpenClaw

Unzip the package to `D:\npm-global\flybook`, then install the skill:

```bash
openclaw skills install <path-to-flybook-skill> --as flybook --force
```

### 3. Configure credentials

Edit `D:\npm-global\flybook\config.json` and fill in your Feishu app credentials:

- `app_id` / `app_secret`: From Feishu Open Platform
- `receive_id_type`: `"user_id"` or `"open_id"`
- `contacts`: Map display names to user IDs

### 4. Add to PATH

Ensure `D:\npm-global\flybook` is in your system PATH so OpenClaw can invoke `flybook.exe`.

### 5. Verify

Restart OpenClaw TUI, then say:

> 给飞书发消息：hello world

## Important notes

- Execute from the flybook directory: `D:\npm-global\flybook\flybook.exe send ...`
- Never guess file paths — confirm the file exists before invoking
- If the contact name is not in config.json, ask the user for their user_id or open_id
- Token is cached at `.token_cache` next to the exe (auto-refreshed)

## Troubleshooting

| Error | Fix |
|-------|-----|
| `No API key found` | Check OpenClaw model provider config |
| `Cannot open config file` | Verify config.json exists next to flybook.exe |
| `unknown contact` | Add contact to config.json or use user_id directly |
| `App does not enable bot feature` | Enable bot capability + publish new version in Feishu Open Platform |
| UTF-8 garbled text | Use ASCII contact alias or pass user_id directly |
