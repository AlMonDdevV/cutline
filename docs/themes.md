# CUTLINE themes

A theme changes how all of CUTLINE looks:
- surface, text, accent and status colours
- timeline colours: clips, playhead, waveforms
- icon colour
- fonts
- corner radius and spacing density

Themes are small JSON files. You can make one in a text editor, test it live in CUTLINE, and share it.

- **Format:** `cutline.theme`, version 1
- **Schema:** [`schema/cutline-theme-v1.schema.json`](../schema/cutline-theme-v1.schema.json) (JSON Schema draft 2020-12)
- **Example files:**
  - [`themes/starter.json`](../themes/starter.json): copy this to start
  - [`themes/cutline-dark.json`](../themes/cutline-dark.json) and [`themes/cutline-light.json`](../themes/cutline-light.json): the built-in themes with every value written out

The schema, the example files and the token table below are generated from one list by
`tools/gen_theme_assets.py`, so they always agree with each other. CUTLINE enforces the same rules when it imports
a theme.

## The file

```json
{
  "format": "cutline.theme",
  "version": 1,
  "name": "My Theme",
  "author": "Your name",
  "description": "Starts from Dark. Change any value; delete the ones you don't need.",
  "base": "dark",
  "colors": { "accent": "#3D8BFD", "focusRing": "#3D8BFD" },
  "fonts": { "ui": "Inter", "mono": "JetBrains Mono" },
  "metrics": { "radius": 4, "density": 1.0 }
}
```

| Field | Required | Rules |
|---|---|---|
| `format` | yes | `"cutline.theme"` |
| `version` | yes | `1`. A file with a higher version is refused with "was made for a newer version of CUTLINE". |
| `name` | yes | 1–40 characters. No `\ / : * ? " < > \|` or control characters. Can't be "Dark", "Light" or "Match System" (any capitalisation). |
| `author` | no | Up to 60 characters. |
| `description` | no | Up to 200 characters. |
| `base` | no | `"dark"` or `"light"`. Every value you leave out comes from that built-in theme. Without `base`, every required colour must be present. |
| `colors` | no (yes without `base`) | Token → `#RRGGBB` or `#AARRGGBB` (alpha first). Only the tokens below. |
| `fonts.ui`, `fonts.mono` | no | Font family names only: letters, numbers, spaces, `-` and `_`, up to 64 characters. Never a path, URL or font data. If the family isn't installed, CUTLINE falls back to its bundled Inter / JetBrains Mono. |
| `metrics.radius` | no | Whole number from 0 to 16: corner radius in pixels (default 3). |
| `metrics.density` | no | 0.8 to 1.25: multiplies spacing and row heights, not text size (default 1). |

**CUTLINE vs the website:**
- **CUTLINE** ignores fields it doesn't know, so a theme made for a newer version still opens in an older one as long
  as `version` is 1.
- **The website's upload check** uses the strict schema, which rejects unknown fields.

## Tokens

<!-- tokens:start -->
| Token | Group | What it colours | Required without `base` |
|---|---|---|---|
| `window` | surfaces | **Window background.** Behind all panels and in the gaps between them. | yes |
| `panel` | surfaces | **Panel background.** The main background of every panel. | yes |
| `panelRaised` | surfaces | **Raised surface.** Menus, headers, tab strips, toolbars and track headers. | yes |
| `field` | surfaces | **Field.** Text fields, drop-downs and buttons. | yes |
| `divider` | surfaces | **Divider.** The 1px lines between panels and rows. | yes |
| `hover` | surfaces | **Hover.** Background under the mouse pointer. | yes |
| `pressed` | surfaces | **Pressed.** Background of pressed or switched-on controls. | yes |
| `selection` | surfaces | **Selection.** Selected rows and selected text. Usually the accent with transparency (#AARRGGBB). | yes |
| `viewerSurround` | surfaces | **Viewer surround.** Around the video in the viewer. Keep it a dark neutral grey so colours are judged correctly. | yes |
| `text` | text | **Text.** Main text. | yes |
| `textMuted` | text | **Muted text.** Labels, secondary text and inactive tabs. | yes |
| `textDisabled` | text | **Disabled text.** Text on disabled controls. | yes |
| `textOnAccent` | text | **Text on accent.** Text drawn on top of the accent colour. | yes |
| `timecodeDim` | text | **Timecode leading zeros.** The dimmed leading zeros in timecode displays. | yes |
| `accent` | accent | **Accent.** Selection marks, the active tab, focus and the logo's cut line. | yes |
| `accentHover` | accent | **Accent hover.** The accent under the mouse pointer. | yes |
| `focusRing` | accent | **Focus ring.** Keyboard focus outlines and the focused panel's top line. | yes |
| `icon` | accent | **Icons.** Icon colour on tool buttons. Optional: defaults to Muted text. | no |
| `clipVideo` | timeline | **Video.** Video track markers and video clips. | yes |
| `clipAudio` | timeline | **Audio.** Audio track markers and audio clips. | yes |
| `clipTitle` | timeline | **Titles.** Title and text clips (used from the effects preview onwards). | yes |
| `clipGenerator` | timeline | **Generators.** Generated clips such as solid colours (used from the effects preview onwards). | yes |
| `playhead` | timeline | **Playhead.** The playhead line and its marker on the ruler. Optional: defaults to Accent. | no |
| `waveform` | timeline | **Waveform.** Audio waveforms inside clips (used from the import preview onwards). Optional: defaults to Audio. | no |
| `danger` | status | **Danger.** Errors and destructive actions. Always shown with an icon and words too. | yes |
| `warning` | status | **Warning.** Warnings in the status bar and dialogs. | yes |
| `success` | status | **Success.** Confirmations. | yes |
<!-- tokens:end -->

## Make, apply and test a theme

1. **Make it.** Copy [`themes/starter.json`](../themes/starter.json), give it a new `name`, and change the colours you
   want. With `"base": "dark"` or `"light"` you only list what's different.
2. **Import it.** In CUTLINE, open **Edit ▸ Settings… (Ctrl+,) ▸ General ▸ Import Theme…** and pick your file.
   - CUTLINE checks the file. If it's broken, it says exactly what's wrong (for example
     `Theme "My Theme" has an invalid colour for "accent".`).
   - Once it's accepted, CUTLINE copies it into your themes folder and switches to it straight away.
   - It also warns, without blocking, if text may be hard to read or the panels are tinted.
3. **Test it live.**
   - Click **Open Themes Folder** on the same page and edit `My Theme.json` there.
   - Every time you save, CUTLINE updates instantly.
   - If a save has a mistake, CUTLINE keeps showing the last good version and puts the error in the status bar.
   - Switch between themes under **View ▸ Theme** or **Settings ▸ General ▸ Theme**.
4. **Share it.** Use **Export Current Theme…**, or share the file from your themes folder. Upload it to the theme
   library on the CUTLINE website.

### Where themes live

| System | Themes folder |
|---|---|
| Windows | `%LOCALAPPDATA%\CUTLINE\themes` |
| macOS | `~/Library/Preferences/CUTLINE/themes` |
| Linux | `~/.config/CUTLINE/themes` |

Any `.json` file in that folder appears in the theme list (CUTLINE notices new files while it's running).

## Good themes

- Keep `viewerSurround` a dark neutral grey, so footage looks the same whatever theme is on.
- Keep panels neutral grey too if you colour-correct footage. CUTLINE warns when a theme tints them, because a
  tinted interface throws off how you see colour.
- Aim for a contrast of 4.5:1 for `text` and `textMuted` on `window`, `panel`, `panelRaised` and `field`, and 3:1 for
  `accent` on `panel`. CUTLINE warns when a theme falls below these.
- Errors always come with an icon and words, so a red accent and a red `danger` colour can't be confused.

## What's coming

Later previews add tokens for markers, keyframes, transitions, meters and scopes, using the same rules. A theme
written for version 1 keeps working: new tokens follow sensible partners when a theme leaves them out.
