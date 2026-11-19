# The `.cutline` project format (version 1)

A CUTLINE project is one UTF-8 JSON file with the extension `.cutline`.

- **Writing:** the file is written atomically. CUTLINE writes a temporary file next to it and renames it into
  place, so a crash or a full disk never leaves a half-written project.
- **Stable output:** keys are sorted, so saving the same project twice gives identical bytes.

## Top level

| Field | Type | Meaning |
|---|---|---|
| `format` | string | always `"cutline.project"`. Any other value is "not a CUTLINE project" |
| `version` | number | format version, currently `1` |
| `createdWith` | string | the app that last wrote the file, e.g. `"CUTLINE 0.1.0"` |
| `id` | string | project id (UUID) |
| `name` | string | project name, 1–120 characters |
| `settings` | object | `{ "defaultSequence": "<sequence id>" }` (optional; defaults to the first sequence) |
| `media` | array | media items. Empty until Milestone 3 |
| `bins` | array | bin folders. Empty until Milestone 3 |
| `sequences` | array | at least one sequence (below) |
| `ui` | object | reserved for view state |

## Sequence

| Field | Type | Meaning |
|---|---|---|
| `id` | string | unique id |
| `name` | string | 1–120 characters |
| `format.width`, `format.height` | number | even, 16–8192 |
| `format.frameRate` | `[num, den]` | e.g. `[30000, 1001]` for 29.97; 1–240 fps |
| `format.sampleRate` | number | 44100, 48000 or 96000 |
| `format.channels` | number | 2 (stereo) |
| `timecodeStartHour` | number | 0 or 1 (optional, default 0) |
| `tracks` | array | at least one video and one audio track |
| `markers` | array | reserved (Milestone 5) |

Tracks may appear in any order in the file. CUTLINE reads them as video tracks first, then audio, each keeping its
order. The first video track is V1, the bottom layer.

## Track

| Field | Type | Meaning |
|---|---|---|
| `id` | string | unique id |
| `kind` | string | `"video"` or `"audio"` |
| `name` | string | 1–64 characters |
| `locked` | boolean | locked tracks can't be edited |
| `enabled` | boolean | video: visible in the output; audio: not muted |
| `solo` | boolean | audio only (ignored on video) |
| `clips` | array | reserved (Milestone 4) |

## Time

Clips (from Milestone 4) store positions as integer **ticks**, where 1 tick = 1/705,600,000 s. Every common frame
rate, including 23.976, 29.97 and 59.94, and every 44.1/48 kHz sample is a whole number of ticks, so edits never
drift.

## Compatibility

- **Before 1.0,** the format only grows by adding fields that have defaults. The version stays `1`.
- **Unknown fields** are ignored, so older 0.x builds can open files from newer 0.x builds.
- **A file with a higher `version`** is refused with "This project was made with a newer version of CUTLINE
  (project format N). Update CUTLINE to open it." It is never rewritten.
- **Errors** name the exact field, e.g. `The project file is missing sequences[0].format.width.` or
  `sequences[0].format: Width and height must be even numbers between 16 and 8192.`
- **When the format freezes at 1.0,** a JSON Schema will be published and step-by-step migrations will handle
  later versions.
