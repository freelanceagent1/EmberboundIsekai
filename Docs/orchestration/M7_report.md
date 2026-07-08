# M7 Report - Camp Defense Event

Date: 2026-07-08

## Built
- Added `UDefenseEventManager` world subsystem for in-camp defense starts from Watchtower UI, camp HUD, and debug console.
- Campfire gains temporary event health (`200 + 50 * campfire level`) with a defense HUD health bar; campfire death ends in defeat without travel or hard failure.
- Added three-lane wave spawning inside `L_Camp` using pooled `AEmberEnemyBase` actors, forced campfire targeting, enemy cap checks, and M3-style elite modifiers for the final goblin.
- Added `AEmberDefenderActor` pooled stationary turrets for assigned defender monsters; they use monster billboard art and fire pooled projectiles at nearest live lane enemy using defender DPS as 1 shot/sec damage.
- Player auto-attack is disabled by default in camp, enabled during defense, and disabled again when the event ends.
- Victory and defeat rewards are banked, saved, and shown in a result panel. Logs include `DefenseStarted`, `DefenseWave n=`, `DefenderSpawned`, and `DefenseResult`.
- Added debug commands: `Ember.StartDefense`, `Ember.DebugAutoDefense 1`, `Ember.DebugAutoDefense 2`, and `Ember.DamageCampfire <n>`.
- Added `Emberbound.M7.DefenseRewardCalculation` automation coverage for watchtower bonus and defeat rounding.

## Verification
- UBT editor target:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10 -NoUBTMakefiles`
  - Result: `Result: Succeeded`.
- UBT runtime target:
  - Command: `Build.bat EmberboundIsekai Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10 -NoUBTMakefiles`
  - Result: `Result: Succeeded`.
- Automation:
  - Command: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Emberbound.M7.DefenseRewardCalculation; Quit" -unattended -nopause -nosplash -stdout`
  - Result: `Test Completed. Result={Success}` and `EXIT CODE: 0`.
- Smoke A, victory:
  - Command: camp map with `Ember.ResetSave,Ember.GrantResources 1000 1000 1000 1000 0,Ember.BuyBuilding watchtower,Ember.GrantShards bone-rat 5,Ember.AssignMonster bone-rat defender,Ember.DebugAutoDefense 1`.
  - Markers: `DefenderSpawned bone-rat`, `DefenseStarted`, all four `DefenseWave n=1..4`, `DefenseResult Victory gold=16 wood=6 soulash=4 essence=3`, and `EMBER: Saved`.
  - Save/reload: camp reboot loaded `Gold=996 Wood=1003 SoulAsh=1004 ... owned=1 assignments=1`.
- Smoke B, defeat:
  - Command: camp map with `Ember.ResetSave,Ember.GrantResources 1000 1000 1000 1000 0,Ember.BuyBuilding watchtower,Ember.GrantShards bone-rat 5,Ember.AssignMonster bone-rat defender,Ember.DebugAutoDefense 2`.
  - Markers: `DefenseStarted`, `DefenseWave n=1`, `DefenseWave n=2`, `DefenseResult Defeat gold=6 wood=2 soulash=1 essence=1`, and `EMBER: Saved`.
  - Save/reload: camp reboot loaded `Gold=986 Wood=999 SoulAsh=1001 ... owned=1 assignments=1`, confirming no travel lock or camp boot failure.

## Deviations
- Headless Smoke B used `Ember.DebugAutoDefense 2` to apply the wave-2 campfire damage internally. This was necessary because `-ExecCmds` cannot wait for wave 2 and then issue `Ember.DamageCampfire 9999` interactively in an unattended process. The public `Ember.DamageCampfire <n>` command is implemented for manual/runtime use.
- Added an unattended-only auto-exit after debug auto-defense results so validation processes can terminate without killing editor processes. This only triggers when `FApp::IsUnattended()` and debug auto-defense is active.
- Fixed a pre-existing unity-build helper-name collision by renaming `CampBuildingPanelWidget.cpp`'s `ResourceLabel` helper.

## Notes
- The worktree already contained untracked `Tools/Validation/` before M7; it was left untouched and unstaged.
