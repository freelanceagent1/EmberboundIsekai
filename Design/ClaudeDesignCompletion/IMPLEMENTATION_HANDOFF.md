# IMPLEMENTATION_HANDOFF.md

Unreal (UE 5.8, UMG + Paper2D) implementation guide for the HTML reference in this directory. **Unreal integration is NOT done** — this documents intent only.

## Screen hierarchy / navigation graph
```
Boot → MainMenu ──Continue──→ Camp (offline-reward dialog if pending, claim once)
        ├─ New Game (confirm) → Camp (fresh save)
        ├─ Settings ⇄ Credits (overlay; returns to opener)
        └─ Quit → platform handoff (no fake transition)
Camp shell (top bar + nav rail) tabs:
  Camp · Character · Skills · Arsenal · Artifacts · Pet Training · World · Bestiary · Camp Buildings
World → biome detail → difficulty → PreRunLoadout → Run
Run: Room1..5 → BoonPick (after each clear) → Elite → Boss → LootReveal* → Results → Camp | retry
Retreat (confirm) → Results (60% bank ✓ canonical)
```
Never label anything bare "Upgrades": use Skills / Arsenal / Artifacts / Camp Buildings / Pet Training.

## Reusable UMG widgets
WBP_CurrencyChip, WBP_RarityBadge, WBP_StateChip, WBP_ProposedBadge (dev builds only), WBP_ConfirmDialog (focus-trapped), WBP_Toast, WBP_BuildingCard, WBP_ItemCard (weapon/artifact variants), WBP_PetCard, WBP_BestiaryCard, WBP_BiomeCard, WBP_BoonCard, WBP_LootReveal, WBP_RunHUD (7-pip progress), WBP_DetailPanel (per-tab payloads), WBP_NavRail, WBP_TopBar.

## DataAssets / tables required
- DT_Skills (30 rows: id, branch, tier, reqs[], effectText, costTier) — costs resolve via BALANCE.md revision.
- DT_Weapons (5), DT_Artifacts (20 + slots enum + set table), DT_Pets (9: roles, passives, shard thresholds), DT_Biomes (3: hazards, enemy lists, unlocks), DT_Enemies (21 incl. elites/bosses: telegraphs, drops, resists, tameable), DT_Titles ✓, DT_Buildings ✓ (existing canonical costs), DT_RunBoons ✓ (existing 15).
- VisualMap table: entityId → sprite/flipbook asset path (source paths in ASSET_USAGE_MANIFEST.csv).

## Prototype state → save game
`ebi-expanded-proto-v1` model maps 1:1 onto `EmberboundSave` ✓ extensions: wallet{5}, buildings{4 levels}, skills{set}, weapons{owned, equipped, levels, favs}, artifacts{owned, equipped[slot], levels}, pets{shards, unlocked, levels, xp, asc, assign[role]}, bestiary[discovered], world{completed, mastered}, offline{pending, claimed}. Run state is transient (not saved) except banked results.

## Nine-slice candidates
- `assets/na/ui/receptacle-cell.png` (inventory cell) — margins 6/6/6/6 px at 1×.
- Kenney Fantasy UI Borders panel-000…: margins 16/16/16/16 (only if a decorative frame is approved; recolor to charcoal/plum first).

## Typography tokens
- Heading: NormalFont.ttf ("EbiPixel") — sizes 42/26/20/16 px @1080p-equivalent scale (2.6em/1.25em/1em/.85em on 16px base; 125% mode multiplies base to 20px).
- Body: platform sans (Verdana stack in prototype). Minimum body 11px @ frame scale; all interactive labels ≥12px.

## Color tokens
bg0 #0e0b13 · bg1 #14111a · surface #1d1826 · surfaceHi #262031 · edge #3a3147 · edgeHi #574a72 · cream #f2e8d8 · dim #a99bb8 · faint #6d6280 · ember #ff9e50 · emberDeep #e0623d · emberBg #4a2410 · violet #8f7ddb / #c9a0ff · green #9fe8a0 · cyan #7fd8d8 · gold #ffd97a · danger #ff9d8a on #2c1d1d / #6b3434.
Rarity: common #a99bb8, rare #7fd8d8, epic #c9a0ff, legendary #ffd97a.

## Spacing / radius / focus / motion
Spacing scale 4/6/8/10/12/14/18 px. Radius: chips 12–14, cards 10, panels/dialogs 14, buttons 8. Focus: 3px #ffd97a outline, 2px offset, all focusables. Motion: ebiPop (180–300ms scale-in) for dialogs/loot/boons; ebiFlicker idle for sprites; ALL motion gated behind reduced-motion setting. Touch targets ≥48px primary / ≥44px secondary.

## Paper2D candidates
Every `assets/na/monster/*-sheet.png` and boss Idle sheets → flipbooks named `FB_<Entity>_IdleDown/Up/Left/Right` (4-frame rows, 16×16 or 32×32 grid per pack layout); FX sheets → `FB_FX_Flame`, `FB_FX_Ice`. Import with Paper2D nearest-neighbor settings.

## Safe area & input
Use FDisplayMetrics safe zones (HTML uses padding stand-in + `env(safe-area-inset-*)` recommended for web builds). Landscape primary; management screens must reflow to portrait (nav rail collapses; grids single-column — prototype demonstrates at 400px frame). Directional focus: nav rail ↔ content; skill grid arrow-key map in prototype is the gamepad focus map. Dialogs trap focus and restore to invoker.

## Canonical vs proposed on screen
Canonical (repository ✓): player stats, 5 enemies, Tutorial Dragon, 15 run boons + 60/30/10 roll, 4 buildings + costs + Campfire→Pen prereq, 3 pets' roles/rates, titles, offline formula/cap, banking 100/60, save slot name. Everything else is PROPOSED (badged in review mode; numbers are {tokens} or isolated prototypeOnly samples).

## Unresolved before implementation
See CONTENT_PROPOSAL.md "Open decisions" — plus: final respec pricing, inventory caps (prototype samples 4 weapons / 8 artifacts at loot-full), and whether Defense events consume time or are instant.
