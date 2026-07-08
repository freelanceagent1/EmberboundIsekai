# M8 Report - Mobile Polish, Titles, Audio

Date: 2026-07-08

## Built
- Added a custom UMG virtual joystick widget and wired it into the existing player movement path through `AEmberPlayerCharacter::ApplyVirtualJoystickVector`. WASD keyboard movement remains unchanged.
- Added joystick display support for touch-capable platforms, the `Ember.ShowJoystick` runtime cvar, and a persisted desktop/debug toggle via `Ember.SetJoystick`.
- Wrapped the main camp HUD, expedition HUD, and camp building panel roots in `USafeZone`.
- Added data-driven title assets through `UTitleData` and `UTitleSubsystem`, with save-backed counters and unlocked title IDs.
- Added five title data assets: former-office-drone, truck-kun-survivor, failed-chosen-one, spreadsheet-hero, and genre-savvy-idiot.
- Hooked title passives into offline production, upgrade costs, XP gain, and low-health outgoing player damage.
- Added title unlock notifications and logs using `EMBER: TitleUnlocked <id>`, plus a camp HUD Titles view.
- Added M8 title automation tests for passive math and unlock rules.
- Added `UEmberAudioSubsystem`, generated original placeholder WAV SFX, imported them to `/Game/Emberbound/Generated/SFX`, and played them from fire, hit, pickup, level-up, upgrade-select, boss, defense, and run-result events.
- Added persisted master volume support and `Ember.SetMasterVolume`.
- Added dev-only camp HUD debug entry points and debug commands for title counters, title granting, title effect inspection, volume, and joystick settings.
- Added `Docs/ANDROID_PACKAGING.md` with Android SDK/NDK notes, landscape/package guidance, texture budget notes, joystick/touch-interface notes, packaging steps, and untested Android items.

## Verification
- TDD red check:
  - Initial title automation compile failed before implementation with `fatal error C1083: Cannot open include file: 'Core/TitleSubsystem.h'`.
- UBT editor target:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10`
  - Result: `Result: Succeeded`.
- UBT runtime target:
  - Command: `Build.bat EmberboundIsekai Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10`
  - Result: `Result: Succeeded`; runtime executable produced at `Binaries\Win64\EmberboundIsekai.exe`.
- Automation:
  - Command: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Emberbound.M8; Quit" -unattended -nopause -nosplash -stdout`
  - Result: found two tests; `Emberbound.M8.TitlePassiveMath` and `Emberbound.M8.TitleUnlockRules` both passed with `EXIT CODE: 0`.
- SFX generation:
  - Command: `python Tools/AssetGeneration/generate_sfx.py`
  - Result: `SFX_OK n=11 out=E:\mobilegamer\EmberboundIsekai\Saved\GeneratedSFX`.
- SFX import:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="...\Tools\EditorScripts\import_m8_sfx.py" -stdout -unattended -nopause -nosplash`
  - Result: commandlet exited successfully and 11 sound wave assets exist under `Content/Emberbound/Generated/SFX`.
- Title asset creation:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="...\Tools\EditorScripts\create_m8_title_data.py" -stdout -unattended -nopause -nosplash`
  - Result: commandlet exited successfully and 5 title assets exist under `Content/Emberbound/Data/Titles`.
- Title/effects smoke:
  - Command: camp map with `Ember.ResetSave,Ember.GrantResources 2000 2000 2000 2000 0,Ember.GrantShards slime 5,Ember.AssignMonster slime worker,Ember.SimulateOffline 120,Ember.DebugTitleCounter building 5,Ember.DebugTitleCounter boss 1,Ember.DebugTitleCounter upgrade 10,Ember.SetMasterVolume 0.75,Ember.DebugTitleEffects,quit`.
  - Markers: `MonsterUnlocked slime`, `MonsterAssigned slime role=worker`, `OfflineReward mins=120 capped=0 gold=240 soulash=0`, and four unlock logs for `former-office-drone`, `spreadsheet-hero`, `failed-chosen-one`, and `genre-savvy-idiot`.
  - Effect marker: `EMBER: TitleEffects offline120=126 cost20=19 xp20=21 damageLow=13.20 damageHigh=12.00`.
- Save/reload:
  - Rebooted the camp map after the smoke save.
  - Marker: `EMBER: SaveLoaded Gold=2240 Wood=2000 SoulAsh=2000 campfire=0 forge=0 slimeShards=5 owned=1 assignments=1 titles=4 volume=0.75 joystick=0`.
- Joystick smoke:
  - Command: camp map with `Ember.SetJoystick 1`.
  - Markers: `EMBER: JoystickSetting 1` and `EMBER: JoystickShown`.

## Perf Pass
- Scanned for `GetAllActorsOfClass`, `TActorIterator`, tick usage, object pool usage, projectile logs, and companion/defender spawn paths.
- No `GetAllActorsOfClass` runtime usage was found.
- One `TActorIterator` remains in `DefenseEventManager::FindCampfireActor`; it runs when defense starts rather than per tick.
- New joystick tick work only pushes movement while dragging. Camp and expedition joystick visibility checks run on a 0.5 second timer so runtime debug toggles can be reflected without a restart.
- Pool usage remains in place for defense enemies, defenders, player projectiles, companion/defender projectiles, boss projectiles, and pickups.
- No damage text widget pool or cap change was needed because this project did not contain damage-text widgets in the current code.

## Deviations
- `Ember.ShowJoystick 1` did not reliably apply early enough through this UE command-line path before game mode construction, so `Ember.SetJoystick 1` was added and used for unattended desktop smoke. The cvar still exists for runtime use.
- The SFX import and title asset commandlets exited successfully and produced the expected assets, but their plain `IMPORT_OK` / `M8_TITLE_CONTENT_OK` print markers did not appear in captured commandlet stdout.
- Title passive math was verified deterministically through `Ember.DebugTitleEffects`; a full combat run was not used to separately drive low-health player damage and XP logs.
- Safe-area work was verified structurally in code by `USafeZone` wrappers rather than by captured 16:9 and 20:9 screenshots.
- The debug menu is implemented as dev-only camp HUD debug entry points and console/debug commands, not as a polished tabbed settings panel with individual visible buttons for every action.
- Android SDK validation still reports the local Android setup as unavailable/invalid, so Android packaging and on-device testing remain documented but untested.

## Notes
- All UBT invocations used `-MaxParallelActions=10`.
- Only editor/runtime processes started for M8 smoke were stopped after their markers were collected.
- The pre-existing untracked `Tools/Validation/` directory was left untouched and unstaged.
