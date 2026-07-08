# TECHNICAL_DESIGN.md — Emberbound Isekai

Architecture of the UE 5.8 vertical slice. C++ core systems, data-driven tuning via DataAssets/DataTables, UMG UI, Enhanced Input.

## Module layout

Single runtime module `EmberboundIsekai` (`Source/EmberboundIsekai/`), organized by folders:

```
Source/EmberboundIsekai/
  Core/        GameInstance, SaveManager, EconomyManager, SceneFlowManager,
               DataRegistry, ObjectPoolSubsystem
  Combat/      EmberPlayerCharacter, AutoAttackComponent, HealthComponent,
               XPComponent, LootDropComponent, EmberProjectile, StatusEffects
  Enemies/     EmberEnemyBase, enemy AI (component-based chase/shoot),
               BossTutorialDragon
  Expedition/  ExpeditionGameMode, RoomManager, EnemySpawner, RewardManager,
               RunUpgradeManager
  Camp/        CampGameMode, CampManager, BuildingActor + concrete buildings,
               OfflineRewardManager
  Monsters/    MonsterCollection, assignment logic, CompanionActor, DefenderActor
  Defense/     DefenseWaveManager, CampfireHealth, defense flow
  Data/        UDataAsset types: EnemyData, RunUpgradeData, MonsterData,
               BuildingData, WeaponData, TitleData, ResourceCost struct
  UI/          C++ widget bases; UMG widgets live in Content/Emberbound/UI
```

## Key decisions

- **Subsystems over singletons**: `UGameInstanceSubsystem` for Economy, SaveManager, SceneFlow, DataRegistry, ObjectPool — globally reachable, no God object.
- **Data-driven**: every enemy/upgrade/monster/building/title is a `UPrimaryDataAsset` registered with the DataRegistry (loaded via AssetManager scan of `/Game/Emberbound/Data`).
- **Object pooling**: `UObjectPoolSubsystem` (world subsystem) pools projectiles, pickups, floating damage text, and enemies. Actors implement `IPoolable` (Activate/Deactivate reset contract).
- **Auto-targeting**: `UAutoAttackComponent` re-acquires nearest enemy on a 0.15s timer (not per-tick), using a cached list maintained by the spawner — no world actor iteration per frame.
- **Status effects**: lightweight `FStatusEffect` array on HealthComponent (burn/poison/slow), ticked at 4Hz.
- **Run vs permanent state**: `URunState` (owned by ExpeditionGameMode, dies with the run) vs `UEmberSaveGame` (permanent: resources, building levels, monsters, titles, timestamp).
- **Scene flow**: Camp map ↔ Expedition map via `USceneFlowManager::TravelTo(...)`, carrying run results through the GameInstance.
- **UI**: C++ `UUserWidget` bases expose BlueprintImplementable bind points; widget BPs are created by editor Python automation and bind generated icon textures.
- **Mobile perf**: fixed-interval targeting, pooling, no dynamic shadows (blob shadow material), simple unlit/lit-translucent materials, capped active enemies (24), texture budgets per spec.

## Automation pipeline (no interactive editor needed)

1. Workers author C++ → orchestrator compiles via UBT.
2. Content (maps, DataAssets, input assets, widget BPs, materials) is created by **headless editor Python** (`UnrealEditor-Cmd.exe ... -run=pythonscript -script=...`) under `Tools/EditorScripts/`.
3. Generated art: `Tools/AssetGeneration/` (ComfyUI API client → PNG → Pillow post-process → UE import script).
4. Validation: automation smoke tests + `-game` mode boot runs; manual PIE checklist in TEST_REPORT.md.

## As-built notes (M1–M5b)

- **Combat (M1/M2)**: enemies are `APawn` + `UFloatingPawnMovement` (not Characters) registered with `UEnemySpawnerSubsystem`; auto-attack acquires targets from that registry on a 0.15 s timer. Damage resolves exclusively on projectile hit via swept traces (overlap fallback) — supports pierce, ricochet, split, and status payloads (burn/poison/frost ticked at 4 Hz on `UHealthComponent`). All 15 run upgrades are `URunUpgradeData` assets; `URunUpgradeManager` (world subsystem) rolls rarity-weighted offers (60/30/10) and resets everything at run end.
- **Expedition flow (M1/M3)**: single persistent arena; a "room" is a wave-set (1–5 normal, 6 elite, 7 boss); portal actor advances rooms. Boss cycles FireballVolley → DashCharge (telegraph decal) → FlameRing, enrages <30% HP. Run resources accrue to a run-local tally banked at run end (100% victory / 60% defeat) into `UEconomySubsystem`.
- **Camp & persistence (M4)**: `UEmberSaveGame` (slot "EmberboundSave", versioned) + `USaveManagerSubsystem` with autosaves at camp entry, purchases, and run banking. Buildings are `UBuildingData` assets (IDs `campfire-core`, `grim-forge`, `monster-pen`, `watchtower`); Grim Forge applies a player-damage multiplier at run start. Scene flow is real `OpenLevel` travel with pending-run-result banking on camp entry.
- **Generated art (M5)**: 62 SDXL-generated textures imported to `/Game/Emberbound/Generated/*`; characters/enemies/buildings render as camera-facing billboard quads (masked unlit material + blob shadow) over collision primitives; floors are runtime-spawned planes with generated material instances (headless level-mutation APIs crash in UE 5.8 — see ASSUMPTIONS). UI resolves icons by convention path `/Game/Emberbound/Generated/UIIcons|UpgradeIcons/T_<PascalId>`.
- **Build config**: module uses `NoPCHs` (shared-workstation PCH memory pressure); UBT invocations pass `-MaxParallelActions=10` by policy.
- **Debug/driver hooks** (dev builds): `Ember.GrantXP/GrantResources/BuyBuilding/StartRoom/DamageBoss/KillPlayer/TravelExpedition/TravelCamp/ResetSave`, smoke drivers `Ember.AutoPickUpgrades/DebugAutoGrantXP/DebugAutoDamageBoss/DebugKillPlayerAfter/DebugAutoReturnCamp`.

*Pending expansion: M6 monster/offline math, M7 defense waves, M8 titles/joystick/audio.*
