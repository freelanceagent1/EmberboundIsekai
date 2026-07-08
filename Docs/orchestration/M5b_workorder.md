# WORK ORDER M5b — Import Generated Art into UE and Wire It Into the Game

Continue Emberbound Isekai (UE 5.8). M1–M4 merged; 62 QA-passed generated PNGs live in `GeneratedAssets/processed/<category>/` with manifest `Tools/AssetGeneration/asset_manifest.json` (intended UE paths inside). Same hard rules (UBT green with `-MaxParallelActions=10`, ONE editor process, original content, never kill processes you didn't start).

## Scope
1. **Import**: review/fix `Tools/AssetGeneration/import_to_unreal.py` (written by a non-editor worker, never executed) and run it headlessly: all processed PNGs → `/Game/Emberbound/Generated/<Category>/T_<PascalId>` (+ `_256` variants skipped — import the 512 icon size and the 1024 char/env sizes; keyart 1536). Texture settings: icons/keyart = UserInterface2D, no mips; environment = Default (world), sRGB on. Print `IMPORT_OK n=<count>`; verify-script the full expected list from the manifest.
2. **Materials** under `/Game/Emberbound/Materials/Generated` (editor python, simple graphs): `M_Generated_Icon` (UI/translucent, texture+alpha), `M_Generated_Billboard` (masked two-sided, texture param, alpha mask, unlit), `M_Stylized_Enemy` (extend/replace M1 `M_Enemy` usage: base tint × optional texture), `M_Campfire_Flame` (unlit emissive, campfire-flame texture, subtle UV panner), `M_Corruption_Decal` (deferred decal, corruption texture).
3. **Wire into gameplay (C++ where logic, python where assets)**:
   - Enemies + boss: camera-facing billboard quad (static mesh plane component, MID of `M_Generated_Billboard`) showing their character texture (GreenSlime/RedSlime/GoblinStabber/GoblinArcher/BoneRat/TutorialDragon), keep primitive body as collision only (hide or shrink); add flat blob-shadow disc. Hit-flash now flashes the billboard MID.
   - Player: billboard with `T_RiftArcherFull`.
   - Camp buildings: billboards with their building textures; campfire actor also gets a flame quad using `M_Campfire_Flame`.
   - Floors: expedition floor material → corrupted-grass texture (tiled), camp floor → meadow-grass/camp-dirt.
   - Projectiles: player arrow → `T_ArrowProjectile` billboard; boss fireball → `T_FireProjectile`.
   - UI: HUD resource icons, upgrade-card icons (all 15 via convention path), result-screen header art (`T_RiftArcherPortrait`), camp panel headers, offline popup icon slots (M6 will consume), splash: show `T_MainSplash` full-screen for 2 s on game boot (simple widget, skippable on tap/key).
   - Log `EMBER: IconResolved <resolved>/<expected>` at HUD construct; expected = every icon the UI asked for.
4. **Pickups**: gold/soul-ash pickups get their vfx sprite billboards.

## Verification
1. UBT green (throttled). 2. Import + verify scripts pass (`IMPORT_OK`, all manifest paths exist). 3. Smoke: camp boot + expedition boot; `IconResolved` shows 100% resolution, no missing-texture warnings for Emberbound paths (`LogUObjectGlobals` grep), no crashes. 4. Update ASSET_MANIFEST.md "Used by" column where wiring changed reality; `Docs/orchestration/M5b_report.md`; TEST_REPORT.md rows. 5. Commit `M5b: generated art imported and wired`. Print `M5B_DONE`.
Out of scope: monster panel content (M6), defense (M7), joystick (M8).
