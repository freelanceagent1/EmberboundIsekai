# TEST_REPORT.md

Compile/run status and manual test checklist. Updated every milestone.

## Compile status
| Date | Target | Result |
|---|---|---|
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (empty module skeleton) | ✅ Succeeded (127 s, VS2022 14.44 toolchain) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M1 core combat loop) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M2 run upgrade system) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M3 boss and run results) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |
| 2026-07-08 | EmberboundIsekaiEditor Win64 Development (M4 camp/save/buildings) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekaiEditor Win64 Development (M5b generated art wiring) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekai Win64 Development (M5b runtime build) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekaiEditor Win64 Development (M6 monsters/offline income) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekai Win64 Development (M6 runtime build) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekaiEditor Win64 Development (M7 camp defense event) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekai Win64 Development (M7 runtime build) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekaiEditor Win64 Development (M8 mobile polish, titles, audio) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |
| 2026-07-08 | EmberboundIsekai Win64 Development (M8 runtime build) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT, `-MaxParallelActions=10`) |

## Automated checks
| Date | Check | Result |
|---|---|---|
| 2026-07-07 | M1 content build script | ✅ `CONTENT_OK` |
| 2026-07-07 | M1 asset verify script | ✅ `M1_ASSET_VERIFY_OK` |
| 2026-07-07 | M1 55s headless smoke | ✅ `PlayerSpawned`, `EnemySpawned`, `ProjectileFired`, `RoomCleared`; no `Fatal`/`Error: [C++]` grep hits |
| 2026-07-07 | M2 upgrade content script + asset check | ✅ `CONTENT_OK`; 15/15 upgrade `.uasset` files present |
| 2026-07-07 | M2 65s headless smoke | ✅ 9 `LevelUp`, 9 `UpgradeApplied`, stacked upgrades, 39 `ProjectileHit`, continued `ProjectileFired`; no `Fatal`/`Error: [C++]` grep hits |
| 2026-07-07 | M3 room 7 victory smoke | ✅ `BossPattern FireballVolley`, `DashCharge`, `FlameRing`, `BossEnraged`, `BossDefeated`, `RunResult Victory KeepPct=100 Gold=25 Wood=10 SoulAsh=8 Embers=0 Shards=3`; no `Fatal`/`Error: [C++]` grep hits |
| 2026-07-07 | M3 room 7 defeat smoke | ✅ `RunResult Defeat KeepPct=60 Gold=0 Wood=0 SoulAsh=0 Embers=0 Shards=0`; no `Fatal`/`Error: [C++]` grep hits |
| 2026-07-08 | M4 building/save automation test | ✅ `Emberbound.M4.BuildingUpgradeCost` completed with `Result={Success}` |
| 2026-07-08 | M4 camp content build + verify | ✅ `CONTENT_OK`; `M4_ASSET_VERIFY_OK`; `L_Camp` and 4 building assets verified |
| 2026-07-08 | M4 camp/save two-boot smoke | ✅ First boot upgraded `campfire-core lvl=2` and `grim-forge lvl=2`; second boot loaded `Gold=30 Wood=20 SoulAsh=6 campfire=2 forge=2` |
| 2026-07-08 | M4 full-loop smoke | ✅ Camp → Expedition → boss victory → Return to Camp → `RunBanked Gold=25 Wood=10 SoulAsh=8 Embers=1 Shards=3`; reboot loaded `Gold=55 Wood=30 SoulAsh=14 campfire=2 forge=2`; `PlayerDamage 15.60` verified forge effect |
| 2026-07-08 | M5b import + content verify | ✅ `IMPORT_OK n=62`; `UPGRADE_ICONS_ASSIGNED n=15`; `M5B_ASSET_VERIFY_OK assets=62 materials=10 upgrade_icons=15` |
| 2026-07-08 | M5b generated art automation test | ✅ `Emberbound.M5b.GeneratedArtPaths` completed with `Result={Success}` |
| 2026-07-08 | M5b camp + expedition smoke | ✅ Camp boot clean; Expedition boot logged `EMBER: IconResolved 6/6`; no Emberbound missing-texture/object grep hits |
| 2026-07-08 | M6 monster content build + verify | ✅ `M6_MONSTER_CONTENT_OK assets=3`; `M6_ASSET_VERIFY_OK monsters=3` |
| 2026-07-08 | M6 unlock/worker/offline smoke | ✅ `MonsterUnlocked slime`; `MonsterAssigned slime role=worker`; `OfflineReward mins=120 capped=0 gold=240 soulash=0`; `OfflineReward mins=240 capped=1 gold=480 soulash=0`; popup logged |
| 2026-07-08 | M6 companion expedition smoke | ✅ Assigned slime companion spawned; `CompanionAttack slime damage=6.0`; companion projectile `ProjectileHit ... damage=6.0` |
| 2026-07-08 | M6 save/reload roundtrip | ✅ Reboot loaded `Gold=720 Wood=0 SoulAsh=0 campfire=0 forge=0 slimeShards=5 owned=1 assignments=1` |
| 2026-07-08 | M7 defense reward automation test | ✅ `Emberbound.M7.DefenseRewardCalculation` completed with `Result={Success}` and `EXIT CODE: 0` |
| 2026-07-08 | M7 victory defense smoke | ✅ Bone-rat defender spawned; `DefenseWave n=1..4`; `DefenseResult Victory gold=16 wood=6 soulash=4 essence=3`; `EMBER: Saved` |
| 2026-07-08 | M7 victory save/reload roundtrip | ✅ Reboot loaded `Gold=996 Wood=1003 SoulAsh=1004 ... owned=1 assignments=1` |
| 2026-07-08 | M7 defeat defense smoke | ✅ Wave-2 defeat path logged `DefenseResult Defeat gold=6 wood=2 soulash=1 essence=1`; `EMBER: Saved` |
| 2026-07-08 | M7 defeat save/reload roundtrip | ✅ Reboot loaded `Gold=986 Wood=999 SoulAsh=1001 ... owned=1 assignments=1`; camp remained bootable |
| 2026-07-08 | M8 title automation tests | ✅ `Emberbound.M8.TitlePassiveMath` and `Emberbound.M8.TitleUnlockRules` completed successfully with `EXIT CODE: 0` |
| 2026-07-08 | M8 SFX generation and import | ✅ `SFX_OK n=11`; 11 generated sound wave assets present under `Content/Emberbound/Generated/SFX` |
| 2026-07-08 | M8 title asset generation | ✅ 5 title data assets present under `Content/Emberbound/Data/Titles` |
| 2026-07-08 | M8 titles/effects smoke | ✅ Offline reward plus debug counters unlocked 4 active titles; `TitleEffects offline120=126 cost20=19 xp20=21 damageLow=13.20 damageHigh=12.00` |
| 2026-07-08 | M8 settings save/reload smoke | ✅ Reboot loaded `titles=4 volume=0.75` with resources and monster assignment intact |
| 2026-07-08 | M8 joystick desktop smoke | ✅ `Ember.SetJoystick 1` logged `JoystickSetting 1` and `JoystickShown` |

## Manual PIE checklist
- [ ] Project opens in UE 5.8 without module rebuild errors
- [ ] Camp map loads; HUD shows resources
- [ ] Expedition starts from Camp
- [ ] Joystick/WASD movement works
- [ ] Auto-attack fires at nearest enemy
- [ ] Rooms lock/unlock correctly; 5 rooms + elite + boss
- [ ] Level-up shows 3 upgrade cards; effects apply and stack
- [ ] Tutorial Dragon Lv.3 uses 3+ patterns; win and lose paths both return to Camp
- [ ] Resources persist; save/reload works
- [ ] Campfire and Bow upgrades purchasable after run 1–2
- [ ] Monster shards collect; at least one assignment works
- [ ] Offline reward popup appears (debug time-skip)
- [ ] Camp defense event runs and rewards
- [ ] Generated assets visible in UI and world
- [ ] Titles panel opens from Camp HUD and shows locked/unlocked effects
- [ ] Touch joystick appears on touch platforms or via debug toggle
- [ ] Placeholder SFX play for combat, pickups, upgrades, defense, boss, and run result events

## Known issues
- M1 map generation falls back to duplicating an engine template map because UE 5.8 headless map creation/loading APIs crash natively in this environment.
- M2 removed M1's direct fire-time auto-attack damage path; damage now resolves from projectile hits.
- M2 `Ember.AutoPickUpgrades` selects immediately under pause for headless smoke because this UE 5.8 timer API does not expose paused world timers.
- M3 defeat smoke starts directly at room 7, so there is no pre-boss run tally to retain; log includes `KeepPct=60` and zero banked amounts.
- M4 adds deterministic headless smoke helpers `Ember.BuyBuilding` and `Ember.DebugAutoReturnCamp`; required M4 exec hooks are also present.
- M5b spawns generated floor material planes at runtime instead of mutating level assets because UE 5.8 headless level-edit APIs crashed in this environment.
- M6 companion smoke used a timed editor process started by the verification command and stopped only that PID after required markers were present.
- M7 defeat smoke used `Ember.DebugAutoDefense 2` to trigger wave-2 campfire damage inside the unattended process; `Ember.DamageCampfire <n>` is implemented for interactive/manual use.
- M8 commandlet stdout did not echo the plain SFX/title import success markers, but commandlets exited successfully and direct asset checks verified 11 SFX assets and 5 title assets.
- M8 joystick unattended smoke used persisted `Ember.SetJoystick 1` because `Ember.ShowJoystick 1` did not apply early enough through this UE command-line path before game mode construction.
- M8 Android packaging remains documented but untested because the local Android SDK setup reports invalid/unavailable.
