# M13 Report - 2D orthographic phone-game presentation

## Summary
- Converted the shared gameplay pawn camera from the old angled spring-arm perspective view to a fixed top-down orthographic camera.
- Re-imported generated textures with `EMBER_FORCE_REIMPORT=1`; imported 62 generated assets and regenerated generated materials. `M_Generated_Floor` is now unlit.
- Enlarged generated ground planes to cover the whole orthographic view and tuned corrupted-grass tint down for Expedition.
- Scaled player/enemy/boss sprites up for phone readability after the first M13 capture pass showed they were still too small.

## Presentation settings
- Camera: `AEmberPlayerCharacter` now attaches `UCameraComponent` directly to the root at `(0, 0, 2200)`, rotation `(-90, 0, 0)`, projection `Orthographic`, `OrthoWidth=1900`.
- Spring arm: removed from the player pawn so there is no perspective arm pitch, camera lag, or rotation lag.
- Sprite facing: `EmberGeneratedArt::FaceComponentToPlayerCamera` now uses the active camera rotation direction, not per-object camera-location vectors. This keeps all sprite planes parallel in orthographic capture.
- Ground coverage: Camp meadow and Expedition corrupted-grass planes are runtime-spawned at scale `120 x 120`, above the template floor, no collision. This hides the template checkerboard/floor from gameplay cameras.
- Expedition floor tint: runtime tint changed to `(0.58, 0.54, 0.62, 1.0)` after material/texture toning to reduce saturation.

## Visual validation
- `Docs/validation/shot_camp.png`: orthographic top-down; no perspective trapezoid; meadow grass fills the full frame; no checkerboard/plane edges; camp buildings and player are flat readable sprites.
- `Docs/validation/shot_expedition.png`: orthographic top-down; corrupted grass fills the full frame; no checkerboard/plane edges; player, enemies, and pickups read as flat 2D sprites.
- `Docs/validation/shot_boss.png`: orthographic top-down; corrupted grass fills the full frame; Tutorial Dragon reads as a large flat orange boss sprite; no 3D horizon or perspective depth cue.
- `Docs/validation/packaged_camp.png`: packaged Development build boots to the same flat 2D camp presentation.

## Verification
- UBT editor target: `EmberboundIsekaiEditor Win64 Development`, `-MaxParallelActions=10` -> `Result: Succeeded`.
- UBT game target: `EmberboundIsekai Win64 Development`, `-MaxParallelActions=10` -> `Result: Succeeded`.
- M13 automation: `Emberbound.M13.PhonePresentationDefaults` -> `Result={Success}`.
- Texture/material re-import: `IMPORT_OK n=62`, `UPGRADE_ICONS_ASSIGNED n=15`.
- Final editor autopilot: `Docs/validation/M13_autopilot_final.log` contains `EMBER: AUTOPILOT COMPLETE`.
- Package: `Docs/orchestration/codex_M13_package_20260708_182543.log` -> `BUILD SUCCESSFUL`, `AutomationTool ExitCode=0`. UAT passed `-ubtargs=-MaxParallelActions=10`.
- Packaged boot capture: `Docs/validation/M13_packaged_camp.log` contains `SaveLoaded`, `DataRegistryLoaded Count=30`, `IconResolved 5/5`, and `CaptureFired`.

## Deviations / notes
- Used a strict straight-down orthographic camera rather than a steep angled camera because the user correction prioritized a genuine 2D phone-game read over preserving pseudo-3D standing depth.
- Re-imported all current generated manifest assets, not only the visibly changed character/floor files, to ensure the UE copies were consistent with `GeneratedAssets/processed/`.
- `Build/` and `Packaged/` outputs were regenerated in the workspace. They are build artifacts and were not staged unless already tracked.
