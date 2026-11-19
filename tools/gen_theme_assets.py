"""Generates CUTLINE's theme assets from one token list, so they never drift apart:

  schema/cutline-theme-v1.schema.json   strict JSON Schema (draft 2020-12) for theme files
  themes/cutline-dark.json              the built-in Dark theme, canonical, every value explicit
  themes/cutline-light.json             the built-in Light theme
  themes/starter.json                   a small theme that inherits from Dark: copy it to start
  docs/themes.md (token table only)     between <!-- tokens:start --> and <!-- tokens:end -->

Run from the repository root:  python tools/gen_theme_assets.py
"""
import json
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]

# (key, group, title, description, required-without-base)
TOKENS = [
    ("window", "surfaces", "Window background", "Behind all panels and in the gaps between them.", True),
    ("panel", "surfaces", "Panel background", "The main background of every panel.", True),
    ("panelRaised", "surfaces", "Raised surface", "Menus, headers, tab strips, toolbars and track headers.", True),
    ("field", "surfaces", "Field", "Text fields, drop-downs and buttons.", True),
    ("divider", "surfaces", "Divider", "The 1px lines between panels and rows.", True),
    ("hover", "surfaces", "Hover", "Background under the mouse pointer.", True),
    ("pressed", "surfaces", "Pressed", "Background of pressed or switched-on controls.", True),
    ("selection", "surfaces", "Selection", "Selected rows and selected text. Usually the accent with transparency (#AARRGGBB).", True),
    ("viewerSurround", "surfaces", "Viewer surround", "Around the video in the viewer. Keep it a dark neutral grey so colours are judged correctly.", True),
    ("text", "text", "Text", "Main text.", True),
    ("textMuted", "text", "Muted text", "Labels, secondary text and inactive tabs.", True),
    ("textDisabled", "text", "Disabled text", "Text on disabled controls.", True),
    ("textOnAccent", "text", "Text on accent", "Text drawn on top of the accent colour.", True),
    ("timecodeDim", "text", "Timecode leading zeros", "The dimmed leading zeros in timecode displays.", True),
    ("accent", "accent", "Accent", "Selection marks, the active tab, focus and the logo's cut line.", True),
    ("accentHover", "accent", "Accent hover", "The accent under the mouse pointer.", True),
    ("focusRing", "accent", "Focus ring", "Keyboard focus outlines and the focused panel's top line.", True),
    ("icon", "accent", "Icons", "Icon colour on tool buttons. Optional: defaults to Muted text.", False),
    ("clipVideo", "timeline", "Video", "Video track markers and video clips.", True),
    ("clipAudio", "timeline", "Audio", "Audio track markers and audio clips.", True),
    ("clipTitle", "timeline", "Titles", "Title and text clips (used from the effects preview onwards).", True),
    ("clipGenerator", "timeline", "Generators", "Generated clips such as solid colours (used from the effects preview onwards).", True),
    ("playhead", "timeline", "Playhead", "The playhead line and its marker on the ruler. Optional: defaults to Accent.", False),
    ("waveform", "timeline", "Waveform", "Audio waveforms inside clips (used from the import preview onwards). Optional: defaults to Audio.", False),
    ("danger", "status", "Danger", "Errors and destructive actions. Always shown with an icon and words too.", True),
    ("warning", "status", "Warning", "Warnings in the status bar and dialogs.", True),
    ("success", "status", "Success", "Confirmations.", True),
]
OPTIONAL_FOLLOWS = {"icon": "textMuted", "playhead": "accent", "waveform": "clipAudio"}
HEX = r"^#([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$"
FAMILY = r"^[A-Za-z0-9][A-Za-z0-9 _-]{0,63}$"
NAME_CHARS = r'^[^\\/:*?"<>|\u0000-\u001f]+$'
RESERVED = r"^\s*([Dd][Aa][Rr][Kk]|[Ll][Ii][Gg][Hh][Tt]|[Mm][Aa][Tt][Cc][Hh] [Ss][Yy][Ss][Tt][Ee][Mm])\s*$"
PLAIN_TEXT = r"^[^\u0000-\u001f<>]*$"


def schema():
    colours = {
        key: {"$ref": "#/$defs/colour", "title": title, "description": desc, "x-group": group}
        for key, group, title, desc, _ in TOKENS
    }
    required_colours = [key for key, *_, req in TOKENS if req]
    return {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "$id": "https://github.com/AlMonDdevV/cutline/blob/main/schema/cutline-theme-v1.schema.json",
        "title": "CUTLINE theme (version 1)",
        "description": "A CUTLINE theme file. With \"base\", every colour, font and metric is optional and inherits from the "
                       "built-in Dark or Light theme. Without \"base\", every required colour must be present.",
        "type": "object",
        "additionalProperties": False,
        "required": ["format", "version", "name"],
        "properties": {
            "format": {"const": "cutline.theme", "title": "Format", "description": "Always \"cutline.theme\"."},
            "version": {"const": 1, "title": "Format version", "description": "Theme format version. This schema is version 1."},
            "name": {
                "type": "string", "minLength": 1, "maxLength": 40, "pattern": NAME_CHARS,
                "not": {"pattern": RESERVED},
                "title": "Name",
                "description": "1-40 characters. Can't contain \\ / : * ? \" < > | or control characters, and can't be "
                               "\"Dark\", \"Light\" or \"Match System\" (any capitalisation).",
            },
            "author": {"type": "string", "maxLength": 60, "pattern": PLAIN_TEXT, "title": "Author", "description": "Who made the theme."},
            "description": {"type": "string", "maxLength": 200, "pattern": PLAIN_TEXT, "title": "Description", "description": "One or two sentences."},
            "base": {"enum": ["dark", "light"], "title": "Base theme", "description": "Built-in theme to inherit every value you leave out from."},
            "colors": {
                "type": "object", "additionalProperties": False, "properties": colours,
                "title": "Colours", "description": "#RRGGBB or #AARRGGBB (alpha first) for each token.",
            },
            "fonts": {
                "type": "object", "additionalProperties": False,
                "title": "Fonts",
                "description": "Font family names only (never paths, URLs or font data). If the family isn't installed, "
                               "CUTLINE uses its bundled Inter / JetBrains Mono.",
                "properties": {
                    "ui": {"$ref": "#/$defs/fontFamily", "title": "Interface font", "description": "Menus, panels and dialogs. Default: Inter.", "x-group": "typography"},
                    "mono": {"$ref": "#/$defs/fontFamily", "title": "Timecode font", "description": "Timecode and rulers. A monospaced family works best. Default: JetBrains Mono.", "x-group": "typography"},
                },
            },
            "metrics": {
                "type": "object", "additionalProperties": False,
                "title": "Metrics",
                "properties": {
                    "radius": {"type": "integer", "minimum": 0, "maximum": 16, "title": "Corner radius", "description": "Corner radius of buttons, fields and menus in pixels. Default: 3.", "x-group": "layout"},
                    "density": {"type": "number", "minimum": 0.8, "maximum": 1.25, "title": "Density", "description": "Multiplies spacing and row heights (not text size). 0.8 = compact, 1.25 = roomy. Default: 1.", "x-group": "layout"},
                },
            },
        },
        "if": {"not": {"required": ["base"]}},
        "then": {"required": ["colors"], "properties": {"colors": {"required": required_colours}}},
        "$defs": {
            "colour": {"type": "string", "maxLength": 9, "pattern": HEX},
            "fontFamily": {"type": "string", "maxLength": 64, "pattern": FAMILY},
        },
    }


def canonical(builtin: dict, name: str, description: str) -> dict:
    colours = dict(builtin["colors"])
    for key, partner in OPTIONAL_FOLLOWS.items():
        colours.setdefault(key, colours[partner])
    ordered = {key: colours[key].upper() for key, *_ in TOKENS}
    return {
        "format": "cutline.theme", "version": 1, "name": name, "author": "CUTLINE", "description": description,
        "colors": ordered,
        "fonts": {"ui": "Inter", "mono": "JetBrains Mono"},
        "metrics": {"radius": 3, "density": 1.0},
    }


def check(theme: dict):
    """The same rules the app enforces, so generated files are known-good."""
    assert theme["format"] == "cutline.theme" and theme["version"] == 1
    assert 1 <= len(theme["name"]) <= 40 and re.match(NAME_CHARS, theme["name"]) and not re.match(RESERVED, theme["name"])
    known = {key for key, *_ in TOKENS}
    assert set(theme.get("colors", {})) <= known, set(theme.get("colors", {})) - known
    for value in theme.get("colors", {}).values():
        assert re.match(HEX, value), value
    if "base" not in theme:
        assert {key for key, *_, req in TOKENS if req} <= set(theme["colors"])
    for value in theme.get("fonts", {}).values():
        assert re.match(FAMILY, value), value


def token_table() -> str:
    rows = ["| Token | Group | What it colours | Required without `base` |", "|---|---|---|---|"]
    for key, group, title, desc, req in TOKENS:
        rows.append(f"| `{key}` | {group} | **{title}.** {desc} | {'yes' if req else 'no'} |")
    return "\n".join(rows)


def main():
    dark = json.loads((ROOT / "src/app/theme/dark.json").read_text(encoding="utf-8"))
    light = json.loads((ROOT / "src/app/theme/light.json").read_text(encoding="utf-8"))

    themes = {
        "cutline-dark.json": canonical(dark, "CUTLINE Dark", "The default theme: neutral dark greys with a red accent."),
        "cutline-light.json": canonical(light, "CUTLINE Light", "Neutral light greys with a deeper red accent. The viewer surround stays dark."),
        "starter.json": {
            "format": "cutline.theme", "version": 1, "name": "My Theme", "author": "Your name",
            "description": "Starts from Dark. Change any value; delete the ones you don't need.",
            "base": "dark",
            "colors": {"accent": "#3D8BFD", "accentHover": "#5C9DFD", "focusRing": "#3D8BFD", "selection": "#403D8BFD",
                       "textOnAccent": "#0B0B0B"},
            "fonts": {"ui": "Inter", "mono": "JetBrains Mono"},
            "metrics": {"radius": 4, "density": 1.0},
        },
    }

    (ROOT / "schema").mkdir(exist_ok=True)
    (ROOT / "schema/cutline-theme-v1.schema.json").write_text(json.dumps(schema(), indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    (ROOT / "themes").mkdir(exist_ok=True)
    for filename, theme in themes.items():
        check(theme)
        (ROOT / "themes" / filename).write_text(json.dumps(theme, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    docs = ROOT / "docs/themes.md"
    if docs.exists():
        text = docs.read_text(encoding="utf-8")
        text = re.sub(r"<!-- tokens:start -->.*<!-- tokens:end -->",
                      "<!-- tokens:start -->\n" + token_table() + "\n<!-- tokens:end -->", text, flags=re.S)
        docs.write_text(text, encoding="utf-8")
    print("wrote schema, %d theme files%s" % (len(themes), ", docs table" if docs.exists() else ""))


if __name__ == "__main__":
    main()
