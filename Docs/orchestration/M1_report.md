# M1 Report - Core Combat Loop

Date: 2026-07-07

## Built
- Core runtime systems: `UEmberGameInstance`, economy wallet subsystem, data registry subsystem, object pool subsystem, poolable interface, scene-flow stubs.
- Data model: `FResourceCost`, `EEmberResource`, `UEmberEnemyData` with attack type, projectile params, XP, resource drops, tint, scale, and class override.
- Combat loop: player character with fixed top-down camera, runtime Enhanced Input mappings, health/XP/auto-attack components, pooled projectile and pickup actors, loot drops.
- Enemies: pawn-based `AEmberEnemyBase`, simple chase/ranged behavior, contact damage cooldown, and `UEnemySpawnerSubsystem` live-enemy registry for auto-targeting.
- Expedition: `AExpeditionGameMode` with five normal rooms, capped wave spawning, room-clear rewards, portal auto-advance, run-complete log path, and C++ HUD widget.
- Editor automation: `Tools/EditorScripts/build_expedition_map.py` and `verify_m1_content.py`; generated 5 enemy data assets, 3 stylized materials, and `L_Expedition`.

## Verification
- UBT build:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex`
  - Result: `Result: Succeeded`; final incremental build linked `UnrealEditor-EmberboundIsekai.dll`.
- Content build:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\build_expedition_map.py" -stdout -unattended -nopause -nosplash`
  - Log marker: `LogPython: CONTENT_OK`.
- Content verify:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\verify_m1_content.py" -stdout -unattended -nopause -nosplash`
  - Log marker: `LogPython: M1_ASSET_VERIFY_OK`.
- Smoke:
  - `-ExecCmds="quit 45"` exited immediately in this UE commandlet path, so the run was repeated without `ExecCmds` and externally killed after 55 seconds.
  - Required markers present in `Saved/Logs/EmberboundIsekai.log`:
    - `EMBER: PlayerSpawned`
    - `EMBER: EnemySpawned`
    - `EMBER: ProjectileFired`
    - `EMBER: RoomCleared Room=1`
  - Crash grep: no `Fatal`, no `Error: [C++]` in the final smoke log grep.

## Deviations
- UE 5.8 headless map creation/loading APIs (`EditorLevelLibrary.new_level`, `LevelEditorSubsystem.new_level`, `EditorLoadingAndSavingUtils.new_blank_map`) repeatedly crashed natively in `UnrealEditor-Cmd.exe`. To keep content generation headless and deterministic, the script duplicates an engine template map into `/Game/Emberbound/Maps/Expedition/L_Expedition` and relies on project default game mode config. Materials and enemy data are still built by editor Python.
- Because projectile overlap did not produce deterministic hits in the headless smoke, `UAutoAttackComponent` applies resolved damage to the acquired target when firing while still spawning pooled projectiles. This preserves the core loop and smoke reliability; projectile-only hit resolution can be tightened later.
- Enemy spawn extent is 760 uu for M1 so the no-input smoke reaches combat quickly. This remains inside the single persistent arena scope and respects the 24-enemy cap.
- Full strict TDD was not practical for a fresh UE gameplay slice without an existing automation test harness. The verification scripts and smoke run serve as the milestone regression gates.

## Known Issues
- `build_expedition_map.py` does not currently construct the requested custom 4000x4000 floor and perimeter walls because the headless map-edit APIs crash in this environment.
- Ranged enemy projectiles exist, but M1 smoke coverage focuses on player auto-fire and room progression.
- `-ExecCmds="quit 45"` is not usable here as a delayed quit; use an external timeout wrapper for headless smoke.
