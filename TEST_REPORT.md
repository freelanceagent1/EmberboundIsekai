# TEST_REPORT.md

Compile/run status and manual test checklist. Updated every milestone.

## Compile status
| Date | Target | Result |
|---|---|---|
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (empty module skeleton) | ✅ Succeeded (127 s, VS2022 14.44 toolchain) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M1 core combat loop) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M2 run upgrade system) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M3 boss and run results) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |

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

## Known issues
- M1 map generation falls back to duplicating an engine template map because UE 5.8 headless map creation/loading APIs crash natively in this environment.
- M2 removed M1's direct fire-time auto-attack damage path; damage now resolves from projectile hits.
- M2 `Ember.AutoPickUpgrades` selects immediately under pause for headless smoke because this UE 5.8 timer API does not expose paused world timers.
- M3 defeat smoke starts directly at room 7, so there is no pre-boss run tally to retain; log includes `KeepPct=60` and zero banked amounts.
