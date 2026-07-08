# EMBERBOUND ISEKAI

Mobile-first, top-down, dark-comedy isekai roguelite idle RPG — UE 5.8 vertical slice.

A failed summoned hero wakes beside a magical campfire, clears roguelite dungeon rooms with auto-attacking bow fire, tames monsters, upgrades the camp, earns offline income, and defends the campfire from raids.

## Requirements

- Windows 11, Unreal Engine **5.8** (`C:\Program Files\Epic Games\UE_5.8`)
- Visual Studio 2022+ with C++ game development workload (for source builds)

## Open & Run

1. Double-click `EmberboundIsekai.uproject` (or open via Epic Launcher → UE 5.8).
2. If prompted to rebuild modules, accept (or prebuild: `Engine\Build\BatchFiles\Build.bat EmberboundIsekaiEditor Win64 Development -Project=<path>\EmberboundIsekai.uproject`).
3. Open map `Content/Emberbound/Maps/Camp/L_Camp` and press **Play (PIE)**.

## Controls

| Input | Action |
|---|---|
| WASD / gamepad stick | Move |
| Virtual joystick (touch platforms; desktop: console `Ember.SetJoystick 1`) | Move |
| (automatic) | Auto-attack nearest enemy in range |
| Mouse / touch | UI interaction, upgrade cards (also keys 1/2/3) |

Useful debug console commands (dev builds): `Ember.GrantResources g w s e em`, `Ember.GrantShards <id> <n>`, `Ember.AssignMonster <id> <role>`, `Ember.SimulateOffline <min>`, `Ember.StartDefense`, `Ember.StartRoom <n>`, `Ember.ResetSave`, `Ember.SetMasterVolume 0..1`, `Ember.SetJoystick 0|1`.

## Core loop

Camp → Expedition (rooms → level-up upgrade choices → Tutorial Dragon Lv.3) → rewards → Camp → upgrade Campfire / Forge / Monster Pen → repeat. Offline income accrues from the last save timestamp.

## Systems

See [TECHNICAL_DESIGN.md](TECHNICAL_DESIGN.md). Balance values: [BALANCE.md](BALANCE.md). Generated assets: [ASSET_MANIFEST.md](ASSET_MANIFEST.md). Test status: [TEST_REPORT.md](TEST_REPORT.md). Assumptions: [ASSUMPTIONS.md](ASSUMPTIONS.md).

## Asset pipeline

`Tools/AssetGeneration/README_ComfyUI_AssetPipeline.md` documents the local ComfyUI generation pipeline (probe → generate → post-process → import to `Content/Emberbound/Generated`).

## Android packaging

See [Docs/ANDROID_PACKAGING.md](Docs/ANDROID_PACKAGING.md). Project settings target landscape mobile (`com.emberbound.isekai`); packaging is documented but untested on this machine (no Android SDK installed).

## Known limitations

Tracked in TEST_REPORT.md → "Known issues" (updated each milestone).
