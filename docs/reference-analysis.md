# CUTLINE — Reference Analysis

Milestone 1, part 1. Written 2026-11-18 before any design or code work.

## What was inspected

| File | What it shows |
|---|---|
| `references/ref-1-finalcut-colour-inspector.jpg` (800×500) | Final Cut Pro: video scopes, viewer, effect/transform inspector, multi-lane timeline ("Makoto") |
| `references/ref-2-finalcut-effects-browser.jpg` (1400×788) | Final Cut Pro: 2×2 scopes grid, inspector sections, timeline with the effects browser docked beside it |
| `references/ref-3-resolve-edit-page.jpg` (2000×1107) | DaVinci Resolve 18 Edit page: media pool, dual viewers, inspector, track-based timeline, audio mixer, page bar |

The screenshots are third-party copyrighted images (Apple, Blackmagic Design). They are kept in a local, git-ignored `references/` folder and are not published with this repository; this page records what was learned from them.

These three images are the only CUTLINE references supplied. There is no separate reference folder or
design document for CUTLINE. (`Documents\BUILDZ\Refrance` exists but holds BUILDZ branding and a
glass-style chat mockup for a different project, so it was not used.)

The images are low-to-medium resolution marketing screenshots. Small text (inspector values, track
header counts) was readable in ref 3, partly readable in ref 2, and only approximately readable in ref 1.

## Reference 1 — Final Cut Pro, colour and inspector

- **Layout:** a thin top toolbar; scopes on the left (vectorscope, RGB parade, luma waveform), the viewer
  in the centre and the inspector on the right; the timeline fills the full width below.
- **Inspector:** collapsible sections (Effects, Compositing, Transform, Crop, Distort). Each has an enable
  checkbox. Rows are label on the left and value on the right, with a unit suffix (px, %, °). Sliders sit
  inline, and a keyframe diamond is in a fixed right-hand column. The focused effect row has a yellow
  outline. A "Save Effects Preset" button is at the bottom.
- **Timecode:** large, centred under the viewer, with the play button beside it.
- **Timeline:** clips are filmstrip thumbnails under a name strip. Colour shows the kind of media: blue
  for video, green for music, teal for sound effects. Waveforms are drawn inside audio clips, and the
  curved shapes at clip ends show fades. The playhead is a thin line with a head on the ruler, and
  markers sit on the ruler. The timeline name and duration are centred above it.

## Reference 2 — Final Cut Pro, effects browser

- **Scopes grid:** luma, RGB overlay histogram, vectorscope and RGB parade, each with its own settings
  icon. A colour-space label ("Rec. 709") sits in the panel header.
- **Inspector:** icon tabs (video, colour, audio, info), with the clip name and duration on the right.
  Sections that can be manipulated on the canvas (Transform, Crop, Distort) have an icon at the right of
  the row that switches on handles in the viewer.
- **Effects browser** docked beside the timeline: a category list (All, Basics, Blur, Colour,
  Distortion, Keying, Light, Looks, Masks, Stylize, Text Effects) and a grid of thumbnails, each showing
  the effect applied to a frame. This is the main strength of this screen: you see the result before
  applying it.
- **Timecode:** the leading zeros are dimmed (`00:01:14:21` with `00:0` in grey), so the digits that
  change stand out.
- **Timeline toolbar:** edit modes (connect, insert, append, overwrite) and the tool picker on the left;
  skimming, audio skimming, solo, snapping and browser toggles on the right. The yellow playhead has a
  head marker on the ruler.

## Reference 3 — DaVinci Resolve 18, Edit page

- **Top bar:** panel toggles with an icon and a text label (Media Pool, Effects Library, Edit Index,
  Sound Library; Mixer, Metadata, Inspector). The labels make them easier to find than icon-only
  buttons. The project name is in the centre.
- **Media pool:** a bin tree (Master, then folders such as Drone and Interviews), Smart Bins (Timelines,
  Keywords) and a thumbnail grid with names underneath. The selected item has an orange-red outline.
- **Two viewers:** source on the left and timeline on the right. Each has its own header (zoom, timecode,
  clip name), jog bar and transport (jump to start, step back, play, stop, step forward, loop).
- **Inspector:** text tabs (Video, Audio, Effects, Transition, Image, File); tabs that don't apply are
  dimmed. Sections (Transform, Cropping, Dynamic Zoom, Composite, Speed Change, Stabilization, Lens
  Correction) have a red enable dot on the left, and a keyframe diamond and reset arrow on the right.
  Numeric fields can be dragged. X and Y have a link toggle.
- **Track headers:** a code (V2, V1, A1–A5), name, lock, auto-select, solo (S) and mute (M), clip count
  and channel layout ("2.0"). A large timeline timecode sits above the headers.
- **Clips:** video is blue with thumbnails, audio is green with waveforms, and titles are beige (e.g.
  "Lower 3rd Simple Underline"). Fades are diagonal shading at clip ends. A transition is a labelled box
  over the cut ("Cross Fade"). Curve and keyframe badges sit in clip corners. The playhead is a red line
  through every track.
- **Timeline toolbar,** centred: select, trim, dynamic trim, blade, insert/overwrite/replace, snapping,
  linked selection, position lock, flags and markers, zoom presets and a zoom slider. Monitor volume,
  DIM and a meter are on the right.
- **Mixer** docked to the right of the timeline: one channel strip per track plus the main bus, with EQ
  and pan, S/M buttons, faders on a dB scale and green meters with yellow peaks.
- **Page bar** at the bottom: Media, Cut, Edit, Fusion, Color, Fairlight, Deliver. Each workspace is a
  stage of the job, and the active one is highlighted.

## What makes them work

1. **The footage is the only colour on screen.** The interface is neutral dark grey with no tint. This
   matters for accuracy, not just looks: a tinted interface shifts how you judge colour in the viewer.
2. **Colour carries meaning.** Blue is video, green is audio, beige is titles, teal is effects audio.
   One accent colour (Resolve red, FCP yellow) marks the playhead and selection. Nothing is decorative.
3. **Dense but orderly.** Panels are split by 1px lines, not cards. Inspector rows have the same height,
   labels and values line up, and keyframe and reset controls always sit in the same right-hand columns.
4. **Detail is revealed when needed.** Sections collapse and have enable toggles, and tabs that don't
   apply are dimmed.
5. **Direct manipulation everywhere.** Handles on the canvas, fields you drag, fade handles on clips and
   live effect thumbnails.
6. **Timecode is a first-class element.** It is large, uses fixed-width digits and dims leading zeros.
7. **Workspaces follow the job** (Resolve's page bar). Labelled toggles make panels easy to find.

## What CUTLINE should take, and what it should not

**Take:** neutral, colour-safe greys; colour meaning media type; one accent; 1px-divided dense panels;
the inspector grid with fixed keyframe and reset columns; draggable numeric fields; dimmed leading zeros
in timecode; live effect thumbnails; clips with waveforms and visible fade handles; Resolve-style track
headers (lock, mute, solo); labelled panel toggles; a docked mixer.

**Do not take:**
- **FCP's trackless magnetic timeline.** The spec asks for traditional tracks with locking and muting,
  so CUTLINE uses Resolve's track model.
- **Any product's icons, page names, colours or exact layout.** CUTLINE gets its own icon set, its own
  accent and its own workspace names.
- **Resolve's density as the default.** It suits colourists on 27" screens. CUTLINE has to work at
  1366×768 on an ordinary laptop, so the default layout is less crowded, and panels open when needed.

**Gaps the references don't cover:** a light theme (none of them has one), the keyboard shortcuts
screen, the export queue, the edit-plan approval flow for Claude, and how the layout behaves on small
screens. These need original design, mocked up before they are built.
