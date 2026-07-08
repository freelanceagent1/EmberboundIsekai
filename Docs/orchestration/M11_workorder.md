# WORK ORDER M11 — Package standalone Windows build

Context: produce a runnable **standalone** (outside-the-editor) Windows build of Emberbound Isekai — the concrete "fully functional version" deliverable. M10 (autopilot + damage numbers) is merged.

Same hard rules (never kill foreign processes — only your own PIDs; one editor/UAT job at a time; original content). Packaging is long-running; run it in the background and poll.

## Config prep
- Ensure the three gameplay maps are cooked: add to `Config/DefaultGame.ini` under `[/Script/UnrealEd.ProjectPackagingSettings]` a `+MapsToCook(FilePath="/Game/Emberbound/Maps/Camp/L_Camp")` (and Expedition; Defense runs inside Camp so no separate map). Set `GameDefaultMap` is already `L_Camp`. Confirm `DirectoriesToAlwaysCook` includes `/Game/Emberbound/Generated` and `/Game/Emberbound/Data` so generated textures, SFX, and data assets cook even if only referenced by soft/convention paths at runtime.
- Keep config = **Development** (NOT Shipping) so the dev-only `Ember.*` console commands (incl. `Ember.Autopilot`, `Ember.CaptureAfter`) remain available for the final packaged end-to-end test.

## Package
Run (background, poll for completion):
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
  -project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" ^
  -noP4 -platform=Win64 -clientconfig=Development -cook -build -stage -pak -archive ^
  -archivedirectory="E:\mobilegamer\EmberboundIsekai\Packaged" -unattended -ncompileeditor -utf8output
```
(Adjust flags as needed for UE 5.8. Use `-nop4`. If `-allmaps` is needed to pick up maps, add it. Cap parallelism if the machine is under load, but do NOT kill other users' processes.)

## Fix cook/package errors
Iterate on any cook errors (missing references, uncooked soft refs, redirectors, save-package failures). Common ones: convention-path textures not cooked (fix via DirectoriesToAlwaysCook), runtime-spawned material/mesh refs (ensure hard-referenced or added to cook), SFX. Re-run until the package succeeds and produces `Packaged/Windows/EmberboundIsekai.exe` (or `Packaged/Windows/EmberboundIsekai/…/EmberboundIsekai.exe`).

## Verify
- Confirm the packaged exe exists and note its exact path.
- Launch it headlessly/off-screen to confirm it boots and loads camp content WITHOUT the editor:
  `"<packaged>\EmberboundIsekai.exe" /Game/Emberbound/Maps/Camp/L_Camp -RenderOffScreen -nosplash -stdout -resx=1280 -resy=720 -ExecCmds="Ember.CaptureAfter 8 showui"` — confirm it logs `SaveLoaded`/`DataRegistryLoaded`/HUD `IconResolved` and produces a HighResShot (packaged screenshots land under `%LOCALAPPDATA%\EmberboundIsekai\Saved\Screenshots` or the packaged Saved dir — locate and copy one to `Docs/validation/packaged_camp.png`). Kill only the PID you launched.

## Report
`Docs/orchestration/M11_report.md`: exact packaged exe path, package config, cook fixes made, boot-verify result + screenshot path. Update README with a "Play the packaged build" section. Commit `M11: standalone Windows package`. Print `M11_DONE`.
