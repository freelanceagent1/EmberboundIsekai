# WORK ORDER M6 — Monster Taming, Assignments, Offline Income

Continue Emberbound Isekai (UE 5.8). M1–M5b merged; read prior reports/sources first. Same hard rules (UBT green `-MaxParallelActions=10`, ONE editor process, pooling, original content, never kill foreign processes).

## Scope
### Data
- `UMonsterData : UPrimaryDataAsset` — Id, DisplayName, Description, ShardsToUnlock, allowed roles bitmask (Companion/Worker/Defender), worker output (resource type + per-minute rate), companion attack params, defender DPS, icon/billboard texture soft refs. Python-create 3 assets per BALANCE.md: slime (5 shards; worker 2 Gold/min; companion acid blob 6 dmg @1.2 s), bone-rat (5; worker 1 Gold + 0.5 SoulAsh/min; defender 4 DPS), skeleton-archer (8; companion arrow 8 dmg; defender 7 DPS).

### Systems
- Shard sources: Bone Rat kill 10% bone-rat shard; boss grants 2–3 random shards (M3 already banks `Shards` count — turn it into typed shards); shards persist in save.
- `UMonsterCollectionSubsystem` (GameInstance): shards per monster, owned monsters (auto-unlock at threshold with log + toast widget `EMBER: MonsterUnlocked <id>`), assignments (one role per monster; slot cap = Monster Pen level + 1, campfire ≥2 gates the pen per M4).
- Companion in expedition: assigned companion spawns beside player (generalize M2 `EmberCompanionSlime` into data-driven `AEmberCompanionActor` using MonsterData; skeleton-archer uses pooled arrow projectiles).
- Worker: contributes to offline income rate.
- Defender: stored DPS total exposed for M7 (`GetDefenderPower()`), shown in Watchtower panel.
- Offline income: on camp map load compute `elapsed = UtcNow - save.LastSaveUtc`, produce worker output for min(elapsed, cap=4h × campfire level), credit wallet, autosave, show offline reward popup widget (per-resource lines with icons, Collect button). Debug exec `Ember.SimulateOffline <minutes>` (rewinds saved timestamp then reruns the flow). Log `EMBER: OfflineReward mins=<m> capped=<0|1> gold=... soulash=...`.
- Monster Pen panel becomes real: collection list (icon, name, shards x/y or OWNED), role assignment buttons (Companion/Worker/Defender/None) with slot-cap enforcement, defender power readout.

### Verification
1. UBT green. 2. Content script + verify (3 monster assets). 3. Smoke A: grant shards via debug (`Ember.GrantShards slime 5`), verify `MonsterUnlocked`, assign worker via debug exec, `Ember.SimulateOffline 120`, verify `OfflineReward` math (2 gold/min × 120 = 240 gold, uncapped at campfire 1? cap = 4h — uncapped; also test 600 min capped case), popup logged. 4. Smoke B: assign slime companion, start expedition, verify companion spawns and `EMBER: ProjectileHit`/acid hits from companion (log `EMBER: CompanionAttack`). 5. Save/reload roundtrip keeps shards/owned/assignments. 6. Report + TEST_REPORT rows + commit `M6: monsters and offline income`. Print `M6_DONE`.
Out of scope: defense event (M7), titles (M8).
