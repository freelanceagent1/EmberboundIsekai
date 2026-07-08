# M4 Report - Camp, Buildings, Save/Load, Full Loop

Date: 2026-07-08

## Built
- Added `UBuildingData` primary data assets with hyphenated gameplay IDs, max level 5, four upgrade costs, and per-level effect magnitudes.
- Added `UEmberSaveGame` and `USaveManagerSubsystem` for slot `EmberboundSave`, resource wallet sync, building levels, monster shard/title/assignment/settings fields, save version, UTC timestamp, explicit save/reset, and autosaves on camp entry, purchases, resource grants, and run banking.
- Added camp runtime: `L_Camp`, `ACampGameMode`, primitive camp building actors, expedition portal actor, camp HUD, and building upgrade panel.
- Made scene flow real with camp/expedition `OpenLevel` calls and explicit game-mode URL options.
- Changed expedition run results to remain pending until camp entry; camp entry banks pending rewards and saves.
- Enabled result screen `Return to Camp`.
- Added debug hooks: required `Ember.GrantResources`, `Ember.ResetSave`, `Ember.TravelExpedition`, `Ember.TravelCamp`; plus M4 smoke helpers `Ember.BuyBuilding` and `Ember.DebugAutoReturnCamp`.
- Applied Grim Forge player damage multiplier to auto-attack damage and logged run-start `EMBER: PlayerDamage <n>`.
- Added `Tools/EditorScripts/build_camp_content.py` and `verify_m4_content.py`.
- Added automation test `Emberbound.M4.BuildingUpgradeCost`.

## Verification
- TDD red check:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10 -Force`
  - Result: failed as expected before implementation with missing `Core/SaveManagerSubsystem.h`.
- UBT build:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10`
  - Result: `Result: Succeeded`.
  - Note: one forced rebuild hit Windows paging-file pressure under the required `-MaxParallelActions=10`; rerun succeeded without changing the parallel cap.
- Automation:
  - Command: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Emberbound.M4.BuildingUpgradeCost; Quit" -TestExit="Automation Test Queue Empty"`
  - Result: `Test Completed. Result={Success} Name={BuildingUpgradeCost}`.
- Content build:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\build_camp_content.py" -stdout -unattended -nopause -nosplash`
  - Marker: `LogPython: Warning: CONTENT_OK`.
- Content verify:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\verify_m4_content.py" -stdout -unattended -nopause -nosplash`
  - Marker: `LogPython: Warning: M4_ASSET_VERIFY_OK`.
- Smoke A, two-boot camp/save cycle:
  - First boot command launched `L_Camp` with `Ember.ResetSave`, `Ember.GrantResources 100 30 10 0 0`, two `Ember.BuyBuilding campfire-core`, two `Ember.BuyBuilding grim-forge`, then `quit`.
  - Markers: `BuildingUpgraded campfire-core lvl=2`, `BuildingUpgraded grim-forge lvl=2`, `EMBER: Saved`.
  - Second boot marker: `EMBER: SaveLoaded Gold=30 Wood=20 SoulAsh=6 campfire=2 forge=2`.
- Smoke B, full loop:
  - Camp boot used `Ember.StartRoom 7`, `Ember.DebugAutoDamageBoss 1`, `Ember.DebugAutoReturnCamp 1`, `Ember.TravelExpedition`.
  - Markers: `EMBER: TravelToExpedition`, `EMBER: PlayerDamage 15.60`, `EMBER: BossDefeated`, `EMBER: RunResult Victory KeepPct=100 Gold=25 Wood=10 SoulAsh=8 Embers=1 Shards=3`, `EMBER: TravelToCamp`, `EMBER: RunBanked Gold=25 Wood=10 SoulAsh=8 Embers=1 Shards=3`, `EMBER: Saved`.
  - Reboot marker: `EMBER: SaveLoaded Gold=55 Wood=30 SoulAsh=14 campfire=2 forge=2`.

## Deviations
- Building package names use UE-safe `DA_CampfireCore`, `DA_GrimForge`, `DA_MonsterPen`, and `DA_Watchtower`; the gameplay IDs remain exactly `campfire-core`, `grim-forge`, `monster-pen`, and `watchtower`.
- Added `Ember.BuyBuilding` and `Ember.DebugAutoReturnCamp` as deterministic headless smoke helpers. The requested exec hooks are also present.
- The content marker scripts emit markers with `unreal.log_warning` because this UE commandlet path suppressed normal `print`/`unreal.log` output.
- The full-loop smoke process was stopped after `RunBanked` was observed. Only the editor process started by the smoke command was stopped.
- UE emitted built-in `LogTemp: Error test` lines during game startup. These are engine unified-error self-test logs; the M4 smoke markers completed and no `Fatal` occurred.

## Known Issues
- Camp building visuals are primitive C++ meshes/placeholders. Generated-art import is still M5b scope.
- Monster panel content remains an `M6` stub and defense remains an `M7` stub per M4 scope.
- Camp map content uses the same duplicate-template pattern as M1 because headless blank-map creation APIs are known to crash in this environment.
