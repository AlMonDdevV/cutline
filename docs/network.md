# Network connections

**CUTLINE 0.1 makes no network connections.** There is no telemetry, no crash upload, no analytics and no account.

Later versions will add exactly four connections. Each one is off until you turn it on, and each is listed here
before it ships:

| Connection | When it happens | What is sent |
|---|---|---|
| Anthropic API (`api.anthropic.com`) | Only if you enable the in-app Claude panel with your own API key | the prompts and project details you choose to share, under the permissions in Settings → Claude |
| Cisco OpenH264 download (`ciscobinary.openh264.org`) | Only when you click "Download OpenH264" in Settings | a normal file download request |
| Update check (GitHub Releases API) | Only if you turn on "Check for updates" (off by default) | a request for the latest release number |
| Caption model download (whisper.cpp model file) | Only when you click "Download speech model" for auto captions | a normal file download request. Your audio is never uploaded; captions are made on your computer |

The Claude Desktop / Claude Code integration (MCP) uses a local socket on your own computer, not the network.
