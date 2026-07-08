# WORK ORDER M4 — Camp, Buildings, Save/Load, Full Loop

Continue Emberbound Isekai (UE 5.8). M1–M3 merged — read `Docs/orchestration/M*_report.md` and existing sources first; build on them. Same hard rules (UBT green, headless python content, ONE editor process, pooling/perf, original content). Known environment quirk: headless map-creation APIs crash — duplicate an engine template map like M1 did (`Tools/EditorScripts/build_expedition_map.py` shows the working pattern).

## Scope

### Data
- `UBuildingData : UPrimaryDataAsset` — Id, DisplayName, Description, MaxLevel (5), per-level `FResourceCost` upgrade costs, per-level effect magnitude. Python-create 4 assets under `/Game/Emberbound/Data/Buildings` per BALANCE.md: campfire-core (20g/5w base, effect: offline cap 4h×lvl + safe radius; lvl2 unlocks Monster Pen), grim-forge (15g/2 soul-ash, +15% player damage per level ≥1), monster-pen (25g/5w, +1 assignment slot), watchtower (20g/3w, +10% defender damage). Cost scaling ×1.8 per level, effects linear.

### Save system
- `UEmberSaveGame` — resources (all 5), building levels, monster shards + owned monsters + assignments (fields now; M6 consumes), unlocked titles (fields), last-save UTC timestamp, unlocked flags, settings stub, save version int.
- `USaveManagerSubsystem` (GameInstance subsystem) — slot "EmberboundSave"; Load on GameInstance Init (before first map); autosave on: camp enter, run end banking, any purchase; explicit Save/ResetDebug API. Economy wallet syncs from/to save. Log `EMBER: SaveLoaded <summary>` / `EMBER: Saved`.

### Camp level + buildings
- `L_Camp` map (duplicate-template pattern) under `/Game/Emberbound/Maps/Camp`, `ACampGameMode` (no enemy systems), player walkable, `GameDefaultMap` → L_Camp.
- `ACampBuildingActor` base: mesh/primitive visual + interaction sphere; walking into range (or pressing E / on-screen button) opens its panel. Concrete: `ACampfireCoreActor` (emissive campfire visual + flat safe-zone ring mesh whose radius scales with level), `AGrimForgeActor`, `AMonsterPenActor` (interaction locked until campfire ≥2 — panel says so), `AWatchtowerActor` (panel placeholder "Defense event — M7"), `AExpeditionPortalActor` (confirm dialog → `USceneFlowSubsystem::TravelToExpedition()`).
- SceneFlow becomes real: OpenLevel by soft path both ways; run-result banking happens on camp entry if a finished run is pending; result screen "Return to Camp" now travels.

### Camp UI (same C++ WidgetTree approach as M1–M3)
- Camp HUD: top resource bar (5 resources; icon slots resolve `/Game/Emberbound/Generated/UIIcons/T_<Name>` via soft path at runtime, tint-swatch fallback if unimported), buttons: Expedition, Upgrades, Monsters (stub panel "M6"), Defense (stub "M7"), Save, Reset (confirm) — large touch-friendly.
- Building upgrade panel: name, current level/max, effect line, cost lines with affordability coloring, Upgrade button (disabled when unaffordable/max), close. Log `EMBER: BuildingUpgraded <id> lvl=<n>`.

### Debug hooks
Execs: `Ember.GrantResources <gold> <wood> <soulash> [essence] [embers]`, `Ember.ResetSave`, `Ember.TravelExpedition`, `Ember.TravelCamp`.

### Verification (run + report)
1. UBT green. 2. Content script `CONTENT_OK` + verify script (map + 4 building assets). 3. Smoke A (camp/save cycle): boot L_Camp → `SaveLoaded` logged → auto-driver grants resources, buys campfire lvl2 (`BuildingUpgraded campfire-core lvl=2`) + forge lvl2, saves, quits; SECOND boot: `SaveLoaded` shows campfire=2, forge=2, spent-down resources. 4. Smoke B (full loop): L_Camp → `Ember.TravelExpedition` → verify ExpeditionGameMode boots + `Ember.StartRoom 7` boss kill via debug → run result Victory → auto "return to camp" → banking + autosave logged; reboot → resources persisted. 5. Forge effect check: player damage log reflects +15% next run (log base damage at run start `EMBER: PlayerDamage <n>`). 6. `Docs/orchestration/M4_report.md` + TEST_REPORT.md rows. 7. Commit `M4: camp, buildings, save/load`. Print `M4_DONE`.
Out of scope: monster panels' real content (M6), defense event (M7), generated-art import (M5b), virtual joystick (M8).
