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

*Sections below are expanded as milestones land (M1 combat details, M4 save format, M6 monster/offline math, M7 defense waves).*
