# Keyboard shortcuts

Every shortcut in CUTLINE belongs to an action that works. Menus, tooltips and the **Keyboard Shortcuts** page all
read from the same list, so a shortcut you see is a shortcut that does something. On macOS, Ctrl means ⌘.

## Shortcuts in 0.1.0

| Action | Shortcut | Works while typing in a field? |
|---|---|---|
| New Project | Ctrl+N | yes |
| Open Project… | Ctrl+O | yes |
| Save Project | Ctrl+S | yes |
| Save Project As… | Ctrl+Shift+S | yes |
| Quit CUTLINE | Ctrl+Q (Alt+F4 also closes the window on Windows) | yes |
| Undo | Ctrl+Z | no: the field undoes its own typing |
| Redo | Ctrl+Y, Ctrl+Shift+Z | no |
| Deselect | Escape (fixed) | see "Escape" below |
| Settings… | Ctrl+, | yes |
| Show Inspector | Shift+5 | no |
| Show History | none (add your own) | no |
| Next / Previous Panel | F6 / Shift+F6, Ctrl+Tab / Ctrl+Shift+Tab | yes |
| Next Panel (from Timeline or Viewer) | Tab | only in the Timeline or Viewer |
| Full Screen | F11 (macOS: Ctrl+⌘+F) | yes |
| Add Video Track, Add Audio Track, Remove Selected Track | none (menus) | no |
| Dark Theme, Light Theme, Match System | none (View ▸ Theme) | no |
| Save Workspace As…, Reset Workspace | none (Workspace menu) | no |
| Keyboard Shortcuts…, About CUTLINE | none (Help menu) | no |

More shortcuts (JKL playback, blade, split, ripple delete and the rest) arrive with the features they control.

## How keys are routed

CUTLINE decides what a key press does in this order:

1. **A drag or trim in progress:** Escape cancels it.
2. **Recording a shortcut** on the Keyboard Shortcuts page: the recorder gets every key.
3. **A dialog or menu is open:** Escape closes it, and other keys go to the dialog, never the editor behind it.
4. **Typing in a text field:**
   - Letters, Space, Delete, arrows and Ctrl+A/C/X/V/Z/Y all edit the text.
   - Only commands marked "works while typing" fire.
   - Escape puts back the original text and leaves the field.
5. **Shortcuts for the panel you're in** (Timeline, Viewer, Inspector, History) come first, then global ones.

**Non-US keyboards:**
- Shift+digit shortcuts use the physical number row, so Shift+5 works on AZERTY, QWERTZ and US layouts alike.
- `+` and `-` also accept `=` and the numpad keys.

## Changing shortcuts

Open **Help ▸ Keyboard Shortcuts…** (or **Settings ▸ Keyboard Shortcuts**).

- **Search** by action name ("history") or by an exact key ("ctrl+s").
- **Record:** click ● next to an action and press the new keys. Escape cancels.
- **Conflicts:** if those keys are already used where the action works, CUTLINE says so ("Ctrl+S is already used by
  “Save Project” (Everywhere).") and offers **Reassign**. Reassign moves the keys and removes them from the other
  action.
- **Restore** one action with ↺, or everything with **Restore All Defaults**.
- **Import… / Export…** share a profile as a `.json` file
  (`{"format":"cutline.shortcuts","version":1,"name":…,"overrides":{…}}`).

Changes save immediately to `shortcuts.json` in CUTLINE's settings folder:
- Windows: `%LOCALAPPDATA%\CUTLINE`
- macOS: `~/Library/Preferences/CUTLINE`
- Linux: `~/.config/CUTLINE`

If that file is ever damaged, CUTLINE starts with the defaults and says so in the status bar.
