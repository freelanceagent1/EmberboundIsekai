# M5b Report - Generated Art Import and Wiring

Date: 2026-07-08

## Summary
- Imported 62 QA-passed PNGs from `GeneratedAssets/processed` into `/Game/Emberbound/Generated`.
- Created generated materials in `/Game/Emberbound/Materials/Generated`: icon, billboard, floor, enemy, campfire flame, corruption decal, blob shadow, and camp/expedition floor instances.
- Wired generated art into enemy/boss/player/building/projectile/pickup billboards, campfire flame, HUD/resource icons, upgrade-card icons, run-result header art, building panel headers, and boot splash art.
- Added `EmberGeneratedArt` helpers and `Emberbound.M5b.GeneratedArtPaths` automation coverage.

## Verification
- Import command: `UnrealEditor-Cmd.exe EmberboundIsekai.uproject -run=pythonscript -script=Tools/AssetGeneration/import_to_unreal.py -stdout -unattended -nopause -nosplash`
  - Result: `IMPORT_OK n=62`, `UPGRADE_ICONS_ASSIGNED n=15`.
- Asset verify command: `UnrealEditor-Cmd.exe EmberboundIsekai.uproject -run=pythonscript -script=Tools/EditorScripts/verify_m5b_content.py -stdout -unattended -nopause -nosplash`
  - Result: `M5B_ASSET_VERIFY_OK assets=62 materials=10 upgrade_icons=15`.
- UBT editor target: `Build.bat EmberboundIsekaiEditor Win64 Development -Project=E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject -WaitMutex -MaxParallelActions=10`
  - Result: succeeded.
- UBT game target: `Build.bat EmberboundIsekai Win64 Development -Project=E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject -WaitMutex -MaxParallelActions=10`
  - Result: succeeded.
- Automation: `Automation RunTests Emberbound.M5b.GeneratedArtPaths`
  - Result: `Result={Success}`, exit code 0.
- Camp smoke: `/Game/Emberbound/Maps/Camp/L_Camp?game=/Script/EmberboundIsekai.CampGameMode`
  - Result: exit code 0; no `LogUObjectGlobals.*Emberbound`, `Failed to find object.*Emberbound`, `Missing.*Emberbound`, or `Error:` grep hits.
- Expedition smoke: `/Game/Emberbound/Maps/Expedition/L_Expedition?game=/Script/EmberboundIsekai.ExpeditionGameMode`
  - Result: `EMBER: IconResolved 6/6`; no missing Emberbound asset grep hits.

## Deviations
- Headless map mutation for floor materials was not used. A level-edit script crashed in UE 5.8 level APIs, so M5b now spawns runtime floor planes with generated camp and expedition materials from the camp/expedition game modes.
- `PCHUsage` was switched to `NoPCHs` after repeated shared-workstation PCH virtual-memory failures. Required explicit includes and direct `LoadObject` mesh loads were added so both editor and game targets build.
- The expedition smoke uses a timed editor process started by the verification command and stops only that process after `IconResolved` is observed.
