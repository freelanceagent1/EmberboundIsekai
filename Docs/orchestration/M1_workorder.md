# WORK ORDER M1 — Core Playable Combat

You are the implementing engineer for **Emberbound Isekai**, a UE **5.8** C++ mobile roguelite (top-down auto-shooter). Read `TECHNICAL_DESIGN.md`, `BALANCE.md`, `ASSUMPTIONS.md` at repo root first. The project skeleton compiles. Your job: make the core combat loop playable, verified headlessly, with NO interactive editor.

## Hard rules
- UE 5.8 at `C:\Program Files\Epic Games\UE_5.8`. Iterate until the build is GREEN:
  `"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" EmberboundIsekaiEditor Win64 Development -Project="E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -WaitMutex`
- Editor content is created ONLY via headless editor Python (PythonScriptPlugin is enabled):
  `"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject" -run=pythonscript -script="<abs path>.py" -stdout -unattended -nopause -nosplash`
  Run ONE editor process at a time. First run compiles shaders — be patient (several minutes).
- Original content only. No copyrighted names/assets. No Marketplace deps.
- Mobile perf: no per-frame `GetAllActorsOfClass`; auto-target on a 0.15 s timer against a spawner-maintained registry; pool projectiles & pickups; cap active enemies at 24; simple materials only.
- Match existing code style; keep systems data-driven; components over God classes.

## Deliverables (C++ under `Source/EmberboundIsekai/<Folder>/`)

### Core/
- `UEmberGameInstance` — holds run-result handoff struct; init subsystems.
- `UEconomySubsystem` (GameInstance subsystem) — resource wallet: enum `EEmberResource { Gold, Wood, SoulAsh, MonsterEssence, Embers }`, Add/Spend/Get + change delegate. (Persistence comes in M4.)
- `UEmberDataRegistry` (GameInstance subsystem) — loads all `UPrimaryDataAsset`s under `/Game/Emberbound/Data` at init (AssetRegistry scan), lookup by type+name. Name it exactly `UEmberDataRegistry` to avoid clashing with the engine's DataRegistry plugin.
- `UObjectPoolSubsystem` (World subsystem) — acquire/release pooled actors by class; `IPoolable` interface with `OnAcquired`/`OnReleased`. Pools: projectiles, pickups, damage text (M2), enemies optional.
- `USceneFlowSubsystem` — stub `TravelToExpedition()/TravelToCamp()` (Camp map lands M4).

### Data/
- `FResourceCost` struct (resource→amount map).
- `UEmberEnemyData : UPrimaryDataAsset` — display name, HP, contact damage, move speed, attack type (Melee/Ranged), projectile params, XP reward, resource drop table (probability entries), color tint + scale (for primitive-mesh visuals), enemy class override.

### Combat/
- `AEmberPlayerCharacter` — `ACharacter`, top-down fixed camera (SpringArm pitch −55°, length ~1400, no camera lag on rotation; camera does not rotate with pawn), orient-to-movement. Enhanced Input: create `UInputAction`/`UInputMappingContext` **at runtime in C++** (NewObject in BeginPlay, add WASD + gamepad stick mappings) so no content assets are required; virtual joystick arrives in M8. Stats from BALANCE.md (HP 100, dmg 12, interval 0.7, speed 450, pickup radius 180, crit 5%/×2).
- `UHealthComponent` — max/current HP, `ApplyDamage`, death + damaged delegates, brief hit-flash hook (sets a scalar on dynamic material instance).
- `UAutoAttackComponent` — 0.15 s acquisition timer choosing nearest live enemy within 900 uu from the spawner registry; fires `AEmberProjectile` from pool every AttackInterval toward target; supports damage/interval/crit stats; modifier hooks (pierce count, split, ricochet — fields now, upgrades consume them in M2).
- `UXPComponent` — XP, level, `XPToNext = 10 × 1.35^(level−1)`, OnLevelUp delegate (M2 UI consumes; for M1 just log + heal 10).
- `AEmberProjectile` — pooled; ProjectileMovement or manual tick move; sphere overlap → damage, hit-flash, release to pool; lifetime 2 s; supports pierce counter.
- `AEmberPickup` — pooled; types: XP orb, Gold, Wood, SoulAsh; magnet-lerps to player when within pickup radius; on collect → XPComponent / EconomySubsystem.
- `ULootDropComponent` — on owner death, rolls `UEmberEnemyData` drop table, spawns pickups from pool.

### Enemies/
- `AEmberEnemyBase` — `APawn` + `UFloatingPawnMovement` (cheap; NOT ACharacter), sphere collision, static-mesh visual (engine sphere/cube scaled+tinted via dynamic material of a simple `M_Enemy` created in the content script), init from `UEmberEnemyData`; chase player (move toward, repath by timer 0.25 s, no navmesh needed in open arena); contact damage on overlap with 0.8 s per-target cooldown. Ranged mode: keep ~600 uu distance, fire pooled projectile every 1.6 s.
- Registry: spawner registers/unregisters live enemies with `UEnemySpawnerSubsystem` (world subsystem) — the auto-attack queries this list.

### Expedition/
- `AExpeditionGameMode` — owns run flow. Room sequence (data-driven array in code for now): 5 normal rooms of escalating waves using the 5 enemy types per BALANCE.md. Single persistent arena; a "room" = wave set; when all dead → grant room-clear reward (BALANCE.md), spawn `AExpeditionPortal` (overlap → next room after 0.5 s fade via camera fade). Room 7 (elite/boss) is M3 — for now after room 5 log "RUN COMPLETE" and idle.
- `AEnemySpawner` logic inside game mode or separate actor: timed wave spawns at arena-edge points, respect 24-enemy cap.
- Minimal HUD: C++-only `UUserWidget` subclass building its tree in `NativeOnInitialized` via `WidgetTree` (HP bar, XP bar, level, room counter, resource counters). No .uasset widgets needed.

### Content script `Tools/EditorScripts/build_expedition_map.py`
Headless editor Python that:
1. Creates `/Game/Emberbound/Maps/Expedition/L_Expedition`: 4000×4000 floor (engine plane scaled, simple dark-grass-tint material `M_Floor`), perimeter blocking walls (scaled cubes), directional + sky light, PlayerStart at center-south, WorldSettings gamemode = `AExpeditionGameMode`.
2. Creates `M_Enemy`, `M_Floor`, `M_Projectile` simple materials under `/Game/Emberbound/Materials/Stylized` (vector param "Tint", scalar "Flash").
3. Creates 5 `UEmberEnemyData` assets under `/Game/Emberbound/Data/Enemies` (GreenSlime, RedSlime, GoblinStabber, GoblinArcher, BoneRat) with BALANCE.md stats, distinct tints/scales.
4. Saves everything; prints `CONTENT_OK` on success.
Then update `Config/DefaultEngine.ini`: `GameDefaultMap`/`EditorStartupMap` = L_Expedition (temporary until M4).

### Verification (you must run these and report)
1. UBT build green.
2. Content script run → `CONTENT_OK` in log; then a verify python (`unreal.EditorAssetLibrary.does_asset_exist`) for map/materials/5 data assets.
3. Smoke run: `UnrealEditor-Cmd.exe <uproject> L_Expedition -game -windowed -resx=1280 -resy=720 -ExecCmds="quit 45"` (or timeout-kill after ~90 s); grep log `Saved/Logs/EmberboundIsekai.log` for: no `Fatal`/`Error: [C++]` crashes; presence of your log lines `EMBER: PlayerSpawned`, `EMBER: EnemySpawned`, `EMBER: ProjectileFired`, `EMBER: RoomCleared` (add these under `LogEmberbound`). Auto-fire will hit enemies without input.
4. Append a row to TEST_REPORT.md compile table + fill `Docs/orchestration/M1_report.md`: what was built, verification output snippets, deviations, known issues.
5. `git add -A` and commit: `M1: core combat loop`.

Do NOT start M2 features (upgrade cards UI), M3 (boss), M4 (camp). Stay in scope; prefer simple working over elaborate unfinished.
