# Phase06 Codec Probe

Status: PASS
FFmpeg path: /opt/homebrew/bin/ffmpeg
Version: ffmpeg version 8.1.1 Copyright (c) 2000-2026 the FFmpeg developers

| Format | Decoder | Status |
| --- | --- | --- |
| M4A/AAC | aac | available |
| FLAC | flac | available |
| MP3 | mp3 | available |
| WAV PCM | pcm_s16le | available |
| WAV float | pcm_f32le | available |

Notes:
- Probe uses argv spawning with `shell:false`.
- This proves local development decoder availability only; packaged end-user FFmpeg bundling remains a later packaging requirement.
- Browser audio APIs are not used.
