# ComfyUI Asset Generation Pipeline — Emberbound Isekai

Automated local art pipeline: ComfyUI (SDXL) generation → Pillow post-processing
→ manifest → (later, by a *different* worker) Unreal import. Built for
Milestone 5(a). This worker never launches Unreal; `import_to_unreal.py` is
authored but intentionally never executed here.

## Contents

| File | Purpose |
|---|---|
| `comfy_client.py` | Thin ComfyUI HTTP/WebSocket API client (probe, submit, wait, fetch). |
| `prompts.json` | Every asset's id/category/prompt/negative/size/seed/target sizes. Source of truth. |
| `../ComfyWorkflows/sdxl_txt2img_api.json` | Minimal SDXL txt2img workflow in ComfyUI API-prompt format. |
| `generate_assets.py` | Orchestrator: generation → retry → fallback → post-process → manifest. |
| `procedural_fallback.py` | Pillow-drawn placeholder generator used when ComfyUI generation fails twice. |
| `import_to_unreal.py` | **UE-editor-only** Python. Not run by this worker. See warning in the file header. |
| `asset_manifest.json` | Machine-readable run results (written by `generate_assets.py`). |
| `../../ASSET_MANIFEST.md` | Human-readable table generated from the JSON manifest. |

## Requirements

- ComfyUI 0.23.0+ running locally, default `http://127.0.0.1:8188`.
- Checkpoint `sd_xl_base_1.0.safetensors` present in `E:\ComfyUI\models\checkpoints\`.
- `C:\Python314\python.exe` with `pillow`, `numpy`, `scipy`, `requests`,
  `websocket-client` installed (`pip install pillow numpy scipy requests
  websocket-client`).

## Usage

Run everything (all ~62 assets), from `Tools/AssetGeneration/`:

```
C:\Python314\python.exe generate_assets.py
```

Re-run just one category (e.g. after tweaking a prompt):

```
C:\Python314\python.exe generate_assets.py --category ui-icons
```

Re-run specific assets by id:

```
C:\Python314\python.exe generate_assets.py --ids gold,wood,health
```

Force-regenerate assets that already have a staged PNG (by default, an
existing `GeneratedAssets/staging/<category>/<id>.png` is reused — cheap
resume after an interrupted run — and only re-post-processed):

```
C:\Python314\python.exe generate_assets.py --ids gold --force
```

List every category/id without generating anything:

```
C:\Python314\python.exe generate_assets.py --list
```

Point at a different ComfyUI instance:

```
C:\Python314\python.exe generate_assets.py --server http://127.0.0.1:8188
```

## Pipeline flow

1. **Probe** — `GET /system_stats`, then confirm the checkpoint is visible
   via `GET /object_info/CheckpointLoaderSimple`. Aborts the whole run (not
   individual assets) if the server is unreachable or the checkpoint is
   missing — there is nothing useful to do without it.
2. **Per asset** — build the workflow from `sdxl_txt2img_api.json` with the
   asset's prompt/negative/size/seed, `POST /prompt`, wait for completion via
   the `/ws` WebSocket (falls back to polling `/history/{id}` if the socket
   is unavailable/drops), fetch the PNG via `/view`, save to
   `GeneratedAssets/staging/<category>/<id>.png`.
3. **Retry** — on any failure (HTTP error, execution error, timeout, empty
   output), retry once with `simplified_prompt` (the short first clause of
   the subject description + the same style/background suffix — a much
   shorter, less conflict-prone prompt).
4. **Fallback** — if the retry also fails, `procedural_fallback.py` draws a
   deterministic flat-shaded placeholder (silhouette blob/diamond/structure/
   burst shape from a small "ember" palette, seeded from the asset id) and
   the manifest marks `status: "fallback"`. The batch never aborts for a
   single asset failure.
5. **Post-process** (Pillow + numpy/scipy), category-dependent:
   - **Alpha-keyed** (`characters`, `buildings`, `ui-icons`, `upgrade-icons`,
     `vfx`): magenta background removal keyed on **hue** (not raw RGB
     distance — see "Why hue-based keying" below) restricted to regions
     connected to the image border (`scipy.ndimage.label`, pure-Python BFS
     fallback if scipy is unavailable), soft 1px edge blur, crop to content
     bounding box with padding, center on a square transparent canvas, then
     resize to every entry in `target_sizes`.
   - **`environment`**: no alpha. A cheap "tileable-ish" seam-softening pass
     (`make_tileable_ish`): wrap the image by half its size so the original
     hard edges land in the middle, then blend a blurred copy only along
     that center cross so the wrap seam is softened. Not true frequency-domain
     tiling, just enough for a stylized mobile background layer.
   - **`keyart`**: no alpha. Generated at 1344×768 (SDXL-native bucket),
     stretch-resized to the final 1536×1024 marketing size.
6. **Manifest** — `asset_manifest.json` gets one entry per asset (prompt,
   negative, seed, size, target UE path, usage note, status, source/processed
   file paths, timing). `ASSET_MANIFEST.md` at the repo root is regenerated
   from it every run.

## Prompt design notes / known limitations

The work order's exact style template and negative template are used as
documented, plus a magenta-background suffix for sprite/icon categories.
Two problems showed up in testing against the live SDXL base checkpoint and
were mitigated (not eliminated — base SDXL, no LoRA/fine-tune, so some
per-seed variance is expected):

- **Icon prompts filled the whole 1024×1024 frame** (e.g. a coin pile with
  no visible background at all), leaving nothing for the keyer to remove.
  Fixed by making the magenta-background instruction much more explicit
  ("single isolated object, one item only... large empty magenta margin...")
  and adding pattern/full-bleed terms to the negative prompt
  (`negative_sprite_extra` in `prompts.json`).
- **Full-body character/structure prompts strongly biased the base
  checkpoint toward a front+back "turnaround sheet"** (two views side by
  side) instead of one pose. Squares invite this composition; switching the
  `characters` category to a portrait SDXL-native bucket (832×1216, see
  `CATEGORY_CFG` in the prompt-authoring notes / `prompts.json` width/height
  per asset) removed the extra horizontal room and reliably produced a
  single figure. Final square variants are still produced identically via
  the crop-to-content + square-canvas step, so this only changes the
  *generation* aspect ratio, not any output size.
- **Why hue-based keying, not RGB-distance-to-(255,0,255):** SDXL renders
  "flat magenta background" with its own shadow/highlight gradient rather
  than a flat color. Measured background samples were RGB like
  `(215,73,133)`…`(69,30,54)` — nowhere near pure magenta in RGB space, but
  all within roughly ±35° of magenta's hue. `alpha_key_magenta()` in
  `generate_assets.py` keys on HSV hue (via `PIL.Image.convert("HSV")`) with
  saturation/value floors, which is stable across that lighting gradient.
- **Residual known limitation:** some renders (especially character
  full-body shots) still show a partially non-magenta background region
  (e.g. a flat gray "floor" band behind the figure's feet) that survives
  keying because it fails the saturation floor and isn't hue-connected to
  the magenta region. The crop-to-content step will include that region in
  the final sprite in those cases. This is a base-checkpoint prompt-adherence
  limit, not a pipeline bug — re-running the specific asset id (which reuses
  the same deterministic seed unless `--force` combined with a prompt edit)
  or bumping the seed in `prompts.json` for that one id is the fix if a
  cleaner cutout is needed later.
- **Occasional off-subject generations.** Base SDXL sometimes ignores a
  short/abstract VFX prompt (e.g. "hit spark burst") in favor of a more
  common composition (e.g. an armored character) when combined with the
  heavily fantasy-coded style template. This is a valid (non-erroring)
  generation from the pipeline's perspective — the retry-on-failure logic
  only catches API/timeout/empty-output failures, not semantic mismatches.
  Spot-check `GeneratedAssets/processed/` after a run; regenerate specific
  ids with an edited/simplified prompt or a different seed as a manual
  curation pass if needed.

## Failure / fallback behavior summary

| Situation | Behavior |
|---|---|
| ComfyUI server unreachable | Whole run aborts at the probe step (nothing to generate against). |
| Checkpoint missing from `/object_info` | Whole run aborts at the checkpoint-confirm step. |
| Single asset: HTTP/execution error, timeout, empty output | Retry once with `simplified_prompt`. |
| Single asset: retry also fails | Procedural fallback PNG generated locally; `status: "fallback"` in manifest; batch continues. |
| Single asset: fallback generation itself raises | `status: "failed"` in manifest (only path that should reach this — no network involved); batch continues. |
| Post-processing raises for a staged image | `status: "failed"`, error recorded; batch continues. |

## Re-running after edits

- Edit `prompts.json` directly (or the generator script used to author it —
  not checked in as a permanent tool, `prompts.json` itself is the source of
  truth going forward) for wording/seed changes, then re-run with `--ids` or
  `--category` and `--force` to bypass the staging-file cache for just the
  assets you changed.
- Post-processing-only changes (e.g. tweak `MAGENTA_HUE_TOLERANCE_PIL` in
  `generate_assets.py`) don't need `--force` — delete nothing; re-running
  without `--force` reuses the existing staged PNG and just re-runs
  `post_process()` on it, which is fast (no ComfyUI round trip).

## Never-run reminder

`import_to_unreal.py` imports the `unreal` Python module, which only exists
inside a running UnrealEditor process. It is meant to be invoked later via
`UnrealEditor-Cmd.exe ... -run=pythonscript -script=import_to_unreal.py` by
whichever worker owns the editor. **This worker (asset generation) must
never launch any Unreal process**, per the M5(a) work order.
