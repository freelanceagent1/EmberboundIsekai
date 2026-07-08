# WORK ORDER M7 — Camp Defense Event

Continue Emberbound Isekai (UE 5.8). M1–M6 merged; read prior reports/sources first. Same hard rules (UBT green `-MaxParallelActions=10`, ONE editor process, pooling, original content, never kill foreign processes).

## Scope
Defense runs **inside L_Camp** (no new map): `UDefenseEventManager` (world subsystem or actor spawned by CampGameMode) started from the Watchtower panel button.

- Campfire becomes attackable: temporary `UHealthComponent` (HP 200 + 50×campfire level) with HUD bar during the event. Event ends in defeat if it hits 0 (no hard fail).
- 3 spawn lanes at camp edges (N/E/W of campfire), enemies path straight toward the campfire (reuse `AEmberEnemyBase` + chase logic with campfire as target; they attack player only if player blocks contact — keep it simple: target = campfire always, contact damage applies to whatever they overlap).
- 4 waves with 3 s inter-wave banner: (1) 6 Green/Red Slimes, (2) 8 Bone Rats, (3) 6 Goblin Stabbers + 2 Goblin Archers, (4) 1 Elite Goblin Stabber (M3 elite modifiers) + 4 Bone Rats. Respect enemy cap; pool everything.
- Player fights: enable auto-attack during the event in camp (disable again after).
- Defenders: for each assigned Defender monster spawn an `AEmberDefenderActor` near the campfire — stationary turret-style, fires pooled projectiles at nearest lane enemy using MonsterData defender DPS (convert to damage per shot at 1 shot/s). Uses monster billboard art.
- Rewards: victory = 15 Gold, 6 Wood, 4 Soul Ash, 3 Monster Essence (+10% per watchtower level); defeat = 40% of that, rounded down. Credit + autosave + result toast/panel. Logs: `EMBER: DefenseStarted`, `EMBER: DefenseWave n=<1-4>`, `EMBER: DefenderSpawned <id>`, `EMBER: DefenseResult <Victory|Defeat> gold=...`.
- Watchtower panel: shows defender power (from M6 `GetDefenderPower()`), Start Defense button (disabled during event).
- Debug: `Ember.StartDefense`, `Ember.DebugAutoDefense 1` (auto-runs event headlessly), `Ember.DamageCampfire <n>`.

## Verification
1. UBT green. 2. Smoke A (victory): camp boot, grant shards + assign bone-rat defender via debug, `Ember.StartDefense` + auto-defense driver; verify all 4 wave logs, `DefenderSpawned`, `DefenseResult Victory`, reward banking + `EMBER: Saved`. 3. Smoke B (defeat): `Ember.DamageCampfire 9999` during wave 2 → `DefenseResult Defeat` with 40% reward, camp still functional (no travel/lock). 4. Save/reload roundtrip after each. 5. Report + TEST_REPORT rows + commit `M7: camp defense event`. Print `M7_DONE`.
Out of scope: joystick/titles/audio (M8).
