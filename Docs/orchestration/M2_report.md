# M2 Report - Run Upgrade System

Date: 2026-07-07

## Built
- Fixed M1 double-damage path: `UAutoAttackComponent` no longer applies damage directly on fire. Damage resolves from projectile hit handling only.
- Reworked `AEmberProjectile` with swept pawn-object traces each tick, overlap fallback, pierce/ricochet support, status payload application, and `EMBER: ProjectileHit` logging.
- Added `URunUpgradeData` primary data assets and `Tools/EditorScripts/create_upgrade_data.py` for all 15 M2 upgrades under `/Game/Emberbound/Data/Upgrades`.
- Added `URunUpgradeManager` world subsystem for rarity-weighted distinct offers, stack limits, apply routing, reset/despawn, `Ember.AutoPickUpgrades`, `Ember.DebugAutoGrantXP`, and `Ember.GrantXP`.
- Added C++ upgrade choice widget with 3 cards, rarity coloring, click selection, and keyboard 1/2/3 selection.
- Added gameplay effects for split, pierce, ricochet, rapid fire, heavy arrow, burn, poison, frost, bone shield, healing wisp, pickup magnet, crit focus, companion slime, soul flame trail, and gold rush.
- Extended health with 4 Hz status-effect ticking and enemy move-speed scaling for frost.

## Verification
- UBT build:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex`
  - Result: `Result: Succeeded`; final pass linked `UnrealEditor-EmberboundIsekai.dll`.
- Content build:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\create_upgrade_data.py" -stdout -unattended -nopause -nosplash`
  - Result: `LogPython: CONTENT_OK`.
- Asset exists check:
  - Verified all 15 upgrade `.uasset` files exist in `Content/Emberbound/Data/Upgrades`.
- Smoke:
  - Command: `UnrealEditor.exe ... /Game/Emberbound/Maps/Expedition/L_Expedition -game -nosplash -nullrhi -log -ExecCmds="Ember.AutoPickUpgrades 1,Ember.DebugAutoGrantXP 1"` with external 65 s timeout.
  - Markers: `EMBER: UpgradeCatalogLoaded count=15`, 9 `EMBER: LevelUp`, 9 `EMBER: UpgradeApplied`, 39 `EMBER: ProjectileHit`, 55 `EMBER: ProjectileFired`.
  - Stacking verified in smoke: `piercing-shot stack=2`, `crit-focus stack=3`, `heavy-arrow stack=2`.
  - Crash grep: 0 `Fatal`, 0 `Error: [C++]`.

## Deviations
- `Ember.AutoPickUpgrades` selects immediately when the choice widget appears. UE 5.8's timer parameters in this project do not support ticking world timers while paused, so the requested 0.5 s paused delay was not reliable for headless smoke.
- Real icon art remains out of M2 scope. Upgrade data supports soft icon refs, but the widget currently displays text/rarity color rather than generated M5 icon textures.
- Strict red/green TDD was limited by the existing UE gameplay slice lacking an automation test harness. The milestone gates were UBT, editor content generation, asset existence, and headless smoke log verification.

## Notes
- The final smoke confirms kills/combat progression via projectile hit resolution after removing fire-time direct damage.
- The worktree contained unrelated generated asset and M4/M5 files before the M2 commit; these were intentionally left unstaged.
