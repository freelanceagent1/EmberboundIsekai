# M9 Capture Report

Result: case B, a real in-world rendering regression was present for the runtime floor art. The delayed capture also confirmed case A contributed to the original evidence: one-frame startup capture was too early for gameplay possession/HUD, but the checkerboard floor persisted after settling because the generated camp/expedition floor planes spawned below the template floor.

Fixes made:
- Added dev command `Ember.CaptureAfter <seconds> [showui]`; it schedules against the active gameplay world and writes `HighresScreenshot*.png` through `FScreenshotRequest` so real-RHI offscreen editor-game runs produce deterministic PNGs.
- Added gameplay command-line fallback scheduling for `Ember.CaptureAfter` from `CampGameMode` and `ExpeditionGameMode`, because this editor launch can consume startup `ExecCmds` before module console commands are usable.
- Added expedition command-line fallback for `Ember.StartRoom` and `Ember.AutoPickUpgrades` so validation launches deterministically enter room 7 / auto-pick upgrades.
- Moved generated camp and expedition floor planes above the template checkerboard floor and made them visual-only (`NoCollision`) so existing gameplay collision remains unchanged.

Validation screenshots:
- `Docs/validation/shot_camp.png` - real-RHI delayed camp capture with HUD, generated grass/dirt floor materials, campfire, forge, monster pen, watchtower/portal billboards, and top-down gameplay camera.
- `Docs/validation/shot_expedition.png` - real-RHI delayed expedition capture with HUD, generated corrupted-grass floor material, player billboard, enemy billboards, and top-down gameplay camera.
- `Docs/validation/shot_boss.png` - real-RHI delayed room-7 boss capture with HUD, generated expedition floor, Tutorial Dragon health/name UI, and the Tutorial Dragon billboard on screen before death.

Verification:
- Build green: `Build.bat EmberboundIsekaiEditor Win64 Development -Project=E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject -WaitMutex -MaxParallelActions=10`.
- Captures were launched one editor process at a time with `UnrealEditor.exe ... -game -RenderOffScreen -nosplash -stdout -resx=1280 -resy=720`, no `-unattended`; only the editor PIDs started for each capture were stopped.
