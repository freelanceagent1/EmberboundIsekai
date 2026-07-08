# WORK ORDER M9-capture — Delayed screenshot command + verify in-world generated art renders

Context: M9 validation. All functional markers pass (combat, boss, defense, save/load verified by the orchestrator). BUT the first in-engine `HighResShot` of `L_Camp` (taken one frame after map load via `-ExecCmds`) shows only the **engine default checkerboard floor with a few tiny dark flat shapes** — no visible campfire, no building billboards, and not the top-down gameplay camera. Evidence PNG: `E:\mobilegamer\EmberboundIsekai\Saved\Screenshots\WindowsEditor\HighresScreenshot00000.png`.

Two possibilities to resolve:
(A) The screenshot simply fired too early (before CampGameMode spawned the generated-material floor plane + building billboards and before the player camera possessed). — likely.
(B) A real regression where the camp/expedition floor plane and/or building billboards do not actually render the generated materials in a real-RHI run. — must be ruled out; spec forbids shipping "gray boxes".

Same hard rules (UBT green `-MaxParallelActions=10`, one editor process at a time, never kill foreign processes, original content only).

## Tasks
1. **Add a debug capture command** `Ember.CaptureAfter <seconds> [showui]` (dev builds): sets a timer for `<seconds>` (default 6) then issues `HighResShot 1280x720` (append `showui` when requested so the HUD is included). Implement so it works in a real-RHI `-game` run (use a world timer + `GEngine->Exec(World, TEXT("HighResShot 1280x720"))` or the HighResShot request API). Log `EMBER: CaptureFired file=<name>`.
2. **Investigate rendering** in a real-RHI run of `L_Camp?game=/Script/EmberboundIsekai.CampGameMode` (launch `UnrealEditor.exe ... -game -RenderOffScreen -nosplash -stdout -resx=1280 -resy=720 -ExecCmds="Ember.CaptureAfter 8 showui"`, NO `-unattended`, kill only your own PID after the shot). Inspect the resulting PNG:
   - Confirm the ground uses the generated camp material (not engine checkerboard). If the CampGameMode spawns a runtime floor plane, ensure it spawns before/independent of timing and actually covers the play area with the generated material instance. Fix if the floor plane is missing/mis-scaled/wrong-material.
   - Confirm the campfire, forge, monster pen, watchtower, and expedition portal render their generated billboards (camera-facing quads with the generated textures) at readable size. Fix scale/orientation/material binding if they are the tiny dark shapes seen in the evidence shot.
   - Confirm the top-down gameplay camera is what HighResShot captures (so the framing matches real play). If the player pawn/camera isn't active at capture time, that's fine once the delay is applied — verify.
3. **Capture the settled loop** (real-RHI, delayed via your new command, NO -unattended). Save PNGs into `E:\mobilegamer\EmberboundIsekai\Docs\validation\`:
   - `shot_camp.png` — camp with buildings + HUD (use `showui`).
   - `shot_expedition.png` — `L_Expedition?game=/Script/EmberboundIsekai.ExpeditionGameMode` with `Ember.AutoPickUpgrades 1` — enemies + player billboards + HUD visible.
   - `shot_boss.png` — expedition with `Ember.StartRoom 7,Ember.AutoPickUpgrades 1` — the Tutorial Dragon billboard on screen (capture before it dies; don't auto-damage, or use a longer boss).
   For each, copy the produced `Saved/Screenshots/WindowsEditor/HighresScreenshot*.png` to the target name and clear between runs.
4. If (B) turned out true and you changed rendering code/content, rebuild green and note the fix. If only (A), state that the art renders correctly once settled and no gameplay fix was needed.

## Report
Write `Docs/orchestration/M9capture_report.md`: which case it was (A/B), any fixes, and the three screenshot paths with a one-line description of what each visibly shows (buildings/enemies/boss/HUD/floor material). Commit `M9-capture: delayed screenshot command + in-world art verification`. Print `M9CAP_DONE`.
