# M6 Report - Monster Taming, Assignments, Offline Income

Date: 2026-07-08

## Built
- Added `UMonsterData` primary data assets and generated three monsters: slime, bone-rat, and skeleton-archer.
- Added `UMonsterCollectionSubsystem` for typed shards, ownership auto-unlock, role assignments, slot caps, defender power, debug commands, and offline worker income.
- Converted run shards from a single tutorial-dragon count to typed monster shard maps. Bone Rat kills can drop `bone-rat` shards at 10%; boss victory grants 2-3 random M6 monster shards.
- Added data-driven `AEmberCompanionActor`; assigned companions spawn on expedition start and use pooled projectiles with `EMBER: CompanionAttack`.
- Added offline reward and monster unlock widgets, plus Monster Pen collection/assignment UI and Watchtower defender power display.
- Added `Tools/EditorScripts/create_m6_monster_data.py` and `verify_m6_content.py`.

## Verification
- UBT editor target:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10`
  - Result: `Result: Succeeded`.
- UBT runtime target:
  - Command: `Build.bat EmberboundIsekai Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex -MaxParallelActions=10`
  - Result: `Result: Succeeded`.
- Content script:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\create_m6_monster_data.py" -stdout -unattended -nopause -nosplash`
  - Marker: `M6_MONSTER_CONTENT_OK assets=3`.
- Content verify:
  - Command: `UnrealEditor-Cmd.exe ... -run=pythonscript -script="E:\mobilegamer\EmberboundIsekai\Tools\EditorScripts\verify_m6_content.py" -stdout -unattended -nopause -nosplash`
  - Marker: `M6_ASSET_VERIFY_OK monsters=3`.
- Smoke A, unlock/worker/offline:
  - Command: camp map with `Ember.ResetSave,Ember.GrantShards slime 5,Ember.AssignMonster slime worker,Ember.SimulateOffline 120,quit`.
  - Markers: `MonsterUnlocked slime`, `MonsterAssigned slime role=worker slots=1/1`, `OfflineReward mins=120 capped=0 gold=240 soulash=0`, `OfflineRewardPopup lines=1`.
  - Capped command: camp map with `Ember.SimulateOffline 600,quit`.
  - Marker: `OfflineReward mins=240 capped=1 gold=480 soulash=0`.
- Save/reload roundtrip:
  - Reboot marker after smokes: `SaveLoaded Gold=720 Wood=0 SoulAsh=0 campfire=0 forge=0 slimeShards=5 owned=1 assignments=1`.
- Smoke B, assigned companion:
  - Setup command: camp map with `Ember.AssignMonster slime companion,quit`.
  - Expedition observation command: started `UnrealEditor.exe` on `L_Expedition` with `Ember.StartRoom 1`; stopped only that launched PID after 22 seconds.
  - Markers: `CompanionSpawned slime`, `CompanionAttack slime damage=6.0`, `ProjectileHit ... damage=6.0`.

## Deviations
- The first content script run failed because UE Python would not nativize `SoftObjectPath` into the `TSoftObjectPtr<UTexture2D>` properties; the script now assigns loaded texture assets.
- The first unlock smoke exposed an invalid base `UUserWidget` toast construction; M6 now uses a concrete `UMonsterUnlockToastWidget`.
- The companion smoke used a timed observation window and stopped only the editor process started by that smoke command.
- UE still emits its built-in `LogTemp: Error test` unified-error self-test lines during startup, matching earlier milestone notes.

## Notes
- The worktree already contained untracked kickoff/workorder files before M6; they were left unstaged unless directly part of the M6 implementation/report.
