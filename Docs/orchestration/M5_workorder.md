# WORK ORDER M5(a) — ComfyUI Asset Generation Pipeline (tooling + generation, NO UE import)

Build the automated local asset pipeline for Emberbound Isekai in `E:\mobilegamer\EmberboundIsekai\Tools\AssetGeneration\`. Original art only — dark-comedy cozy-grim isekai style. **Do NOT launch any Unreal editor process** — another worker owns the editor. Write (but do not run) the UE import script.

## Environment (verified)
- ComfyUI 0.23.0 serving at `http://127.0.0.1:8188` (API: `/system_stats`, `/object_info`, `/prompt`, `/history/{id}`, `/view`, WebSocket `/ws?clientId=`).
- Checkpoint `sd_xl_base_1.0.safetensors` is downloading to `E:\ComfyUI\models\checkpoints\` (~6.9 GB; may still be in progress — poll for file-size stability before first generation; if unavailable after waiting ~15 min, write everything, generate procedural fallbacks, and document).
- Python 3.14 at `C:\Python314\python.exe` (host). `pip install pillow requests websocket-client` is allowed.

## Files to create in Tools/AssetGeneration/
1. `comfy_client.py` — API client: probe server, list checkpoints via /object_info, submit workflow (/prompt), await completion via WebSocket with /history fallback + polling, fetch images via /view, timeouts + retries (1 retry with simplified prompt on failure).
2. `prompts.json` — every asset entry: id, category, prompt, negative, width, height, seed, target sizes. Use style template: "stylized low-poly mobile game asset, dark-comedy isekai fantasy, readable silhouette, warm ember lighting, cute grim cozy survival mood, original design, no copyrighted characters, clean shape language, game-ready" (+ "on plain flat magenta background, centered, full subject in frame" for sprites/icons so BG can be keyed out). Negative: "photorealistic, copyrighted character, logo, text, watermark, cluttered, realistic gore, horror gore, extra limbs, deformed hands, unreadable silhouette, muddy colors, overly detailed background". SDXL native 1024x1024 (icons can generate 1024 then downscale).
3. `Tools/ComfyWorkflows/sdxl_txt2img_api.json` — minimal API-format workflow: CheckpointLoaderSimple → CLIPTextEncode ×2 → EmptyLatentImage → KSampler (euler, 25 steps, cfg 7) → VAEDecode → SaveImage. Checkpoint name parameterized at submit time from discovery.
4. `generate_assets.py` — orchestrates: probe → discover checkpoint → iterate prompts.json → save raw PNGs to `GeneratedAssets/staging/<category>/` → post-process to `GeneratedAssets/processed/<category>/` with Pillow: magenta/near-solid BG → alpha key (flood-fill from corners + tolerance) for icons/sprites; center-crop; resize sets (icons 512+256; characters 1024; textures 1024 and make tileable-ish via mirror blend if simple; keyart 1536×1024 gen at 1344×768 then upscale-resize); write/update `asset_manifest.json` (id, prompt, seed, source file, processed files, intended UE path `/Game/Emberbound/Generated/<Category>/T_<Id>`, usage note, status).
5. `import_to_unreal.py` — UE editor Python (to be run LATER by orchestrator via UnrealEditor-Cmd): imports processed PNGs to `/Game/Emberbound/Generated/<Category>`, sets texture settings (UI icons: UserInterface2D compression, no mipmaps; textures: default; keyart: UserInterface2D), creates material instances hooks (documented). DO NOT RUN IT.
6. `README_ComfyUI_AssetPipeline.md` — setup, usage, how to re-run single categories, failure/fallback behavior.
7. `procedural_fallback.py` — Pillow-drawn stylized placeholders (flat-color silhouette shapes + border + label-free) for any asset that fails generation.

## Asset list (ids kebab-case; category prefix)
- characters: rift-archer-full, rift-archer-portrait, slime-companion, bone-rat, skeleton-archer, goblin-stabber, goblin-archer, red-slime, green-slime, tutorial-dragon
- buildings: campfire-core, grim-forge, monster-pen, watchtower, expedition-portal, ritual-circle
- environment (tileable-ish, no alpha): meadow-grass, corrupted-grass, camp-dirt, stone-path, room-wall, firelight-glow, corruption-decal
- ui-icons: gold, wood, soul-ash, monster-essence, embers, health, xp, bow, campfire, monster-shard, defense-shield, reroll, title-badge
- upgrade-icons: split-shot, piercing-shot, ricochet-shot, rapid-fire, heavy-arrow, fire-arrow, poison-arrow, frost-arrow, bone-shield, healing-wisp, pickup-magnet, crit-focus, companion-slime, soul-flame-trail, gold-rush
- vfx: arrow-projectile, fire-projectile, poison-impact, frost-impact, soul-ash-pickup, gold-pickup, hit-spark, death-puff, campfire-flame, boss-telegraph
- keyart: main-splash (failed isekai hero beside magical campfire with slime companion, dark forest background)

## Execution
After authoring everything: if the checkpoint is ready, run the FULL generation (all ~60 assets; ~5-10 s each on RTX 5070 Ti = under 15 min) and post-processing. If interrupted or some fail, retry once simplified, then fallback-generate procedurally, mark status in asset_manifest.json. Print a final summary table (generated / fallback / failed). Update root `ASSET_MANIFEST.md` table from asset_manifest.json.

Report: write `Docs/orchestration/M5a_report.md` with results, timings, any deviations.
