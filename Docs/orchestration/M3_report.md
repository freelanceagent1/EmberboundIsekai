# M3 Report - Boss and Run Results

Date: 2026-07-07

## Built
- Extended expedition ladder to 7 rooms: rooms 1-5 normal, room 6 elite Goblin Stabber pair with runtime HP/damage/scale/tint modifiers and doubled room reward, room 7 Tutorial Dragon boss.
- Added `ABossTutorialDragon` with HP 450, contact/dash/projectile damage, boss HUD HP source, fireball volley, dash telegraph/charge, flame ring, and enrage below 30% HP.
- Added pooled boss projectile use for volley/ring attacks and required logs: `BossPattern`, `BossEnraged`, `BossDefeated`.
- Refactored run resources into `AExpeditionGameMode` run-local tally. Pickups and room rewards bank only at run end: 100% victory, 60% defeat.
- Added `FEmberRunResult` fields for victory state, earned resources, banked resources, and monster shards.
- Added C++ `URunResultWidget` for VICTORY/DEFEAT, loot lines, Run Again, and disabled Return to Camp placeholder.
- Added debug hooks: `Ember.StartRoom`, `Ember.PlayerDamageMult`, `Ember.KillPlayer`, `Ember.DamageBoss`, plus smoke-driver CVars `Ember.DebugAutoDamageBoss` and `Ember.DebugKillPlayerAfter`.

## Verification
- UBT build:
  - Command: `Build.bat EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex`
  - Result: `Result: Succeeded`; final pass linked `UnrealEditor-EmberboundIsekai.dll`.
- Smoke A, victory:
  - Command: `UnrealEditor.exe ... /Game/Emberbound/Maps/Expedition/L_Expedition -game -nosplash -nullrhi -log -ExecCmds="Ember.StartRoom 7,Ember.AutoPickUpgrades 1,Ember.DebugAutoDamageBoss 1"` with external 32 s timeout.
  - Required markers present:
    - `EMBER: BossPattern FireballVolley`
    - `EMBER: BossPattern DashCharge`
    - `EMBER: BossPattern FlameRing`
    - `EMBER: BossEnraged`
    - `EMBER: BossDefeated`
    - `EMBER: RunResult Victory KeepPct=100 Gold=25 Wood=10 SoulAsh=8 Embers=0 Shards=3`
  - Crash grep: no `Fatal`, no `Error: [C++]` in marker grep.
- Smoke B, defeat:
  - Command: `UnrealEditor.exe ... /Game/Emberbound/Maps/Expedition/L_Expedition -game -nosplash -nullrhi -log -ExecCmds="Ember.StartRoom 7,Ember.DebugKillPlayerAfter 10"` with external 45 s timeout.
  - Required marker present:
    - `EMBER: RunResult Defeat KeepPct=60 Gold=0 Wood=0 SoulAsh=0 Embers=0 Shards=0`
  - Boss remained active through the scheduled debug-kill window and logged patterns before defeat.
  - Crash grep: no `Fatal`, no `Error: [C++]` in marker grep.

## Deviations
- Strict red/green TDD remains limited by the existing UE gameplay slice lacking an automation test harness. I verified the requested M3 markers were absent before implementation, then used UBT plus the required headless smokes as regression gates.
- No new headless Python content script was needed for M3; the boss, telegraph, and result screen are code-only and reuse existing primitive meshes/materials.
- Added two extra smoke-driver CVars, `Ember.DebugAutoDamageBoss` and `Ember.DebugKillPlayerAfter`, to make the requested autonomous headless victory/defeat runs deterministic without an interactive console.
- Defeat smoke starts directly in room 7 as requested, so there is no pre-boss resource tally to retain; the result log includes `KeepPct=60` and zero banked amounts.

## Notes
- The worktree contained unrelated generated asset and M4/M5a files before M3; they were intentionally left unstaged.
- Return to Camp remains disabled per M3 scope and M4 ownership.
