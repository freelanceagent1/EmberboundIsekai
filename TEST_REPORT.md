# TEST_REPORT.md

Compile/run status and manual test checklist. Updated every milestone.

## Compile status
| Date | Target | Result |
|---|---|---|
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (empty module skeleton) | ✅ Succeeded (127 s, VS2022 14.44 toolchain) |
| 2026-07-07 | EmberboundIsekaiEditor Win64 Development (M1 core combat loop) | ✅ Succeeded (`Result: Succeeded`, UE 5.8 UBT) |

## Automated checks
| Date | Check | Result |
|---|---|---|
| 2026-07-07 | M1 content build script | ✅ `CONTENT_OK` |
| 2026-07-07 | M1 asset verify script | ✅ `M1_ASSET_VERIFY_OK` |
| 2026-07-07 | M1 55s headless smoke | ✅ `PlayerSpawned`, `EnemySpawned`, `ProjectileFired`, `RoomCleared`; no `Fatal`/`Error: [C++]` grep hits |

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
- M1 auto-attack applies resolved damage directly on fire for deterministic headless combat; pooled projectiles still spawn/travel.
