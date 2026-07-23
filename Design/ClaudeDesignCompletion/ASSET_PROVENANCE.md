# ASSET_PROVENANCE.md

License summary for everything in `Design/ClaudeDesignCompletion/assets/`. Full per-file rows in `ASSET_USAGE_MANIFEST.csv`.

## CC0 (no attribution required; credited anyway)
- **Ninja Adventure Asset Pack (Update 8)** — Pixel-boy & AAA. Primary gameplay family: all monster/boss facesets & sprite sheets, bow sprites, resource/treasure items, tilesets, skill icons, UI cell, elemental FX sheets, `NormalFont.ttf` (the "EbiPixel" heading font). No pixel edits performed; display uses nearest-neighbor (`image-rendering: pixelated`) at integer-friendly sizes.
- **Kenney packs** — Fantasy UI Borders preview retained as reference only (not rendered in the prototype). UI Pack Pixel Adventure / Input Prompts / Particle Pack: evaluated, none copied — Ninja Adventure's own UI family covered the needs, avoiding competing visual systems.

## CC BY 3.0 (attribution REQUIRED)
- **game-icons.net, contributor `lorc`** — 22 SVGs under `assets/gi/lorc/`. Mapped to artifacts via the `gi` field in `data/proposed.js`. The HTML prototype currently renders matching Unicode glyph placeholders; the copied SVGs are the binding source for final UMG icons and MUST ship recolored (cream `#f2e8d8` on charcoal) with credits:
  `Icons by Lorc (game-icons.net), CC BY 3.0` — plus per-icon filenames listed in the CSV. Modification to declare in final credits: palette recolor only.

## Existing AI key art (project-internal)
- `assets/ebi/*` — copied from `Design/ClaudeDesignBaseline/assets/` (splash, archer portrait, building renders, currency icons). Provenance: project `ASSET_MANIFEST.md` (AI-generated during Milestone 5). Used for key art, camp building cards, and currency chips only; **never mixed with pixel sprites inside a gameplay scene** (run scenes use only Ninja Adventure art).

## New derivatives
- None. No generative AI edits, no repainted frames. Only deterministic renaming and CSS-level tinting/overlays at render time.
