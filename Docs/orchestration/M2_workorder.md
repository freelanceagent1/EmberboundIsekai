# WORK ORDER M2 — Run Upgrade System

Continue engineering Emberbound Isekai (UE 5.8, E:\mobilegamer\EmberboundIsekai). M1 (core combat) is merged: read `Docs/orchestration/M1_report.md` and the existing `Source/EmberboundIsekai/` code first, and build on it — do not rewrite working systems. Same hard rules as M1 (UBT green loop, headless python for content, one editor process, mobile perf, original content).

## PREREQUISITE FIX (mandatory, before upgrade features)
M1 shipped a **double-damage bug**: `UAutoAttackComponent::FireAtTarget` applies damage directly to the target (AutoAttackComponent.cpp:87) AND the pooled projectile applies damage again on overlap (EmberProjectile.cpp:84). Fix properly:
1. Remove the direct `ApplyDamage` on fire. Damage resolves ONLY on projectile hit (crit resolved at fire time is fine to carry on the projectile).
2. Make projectile hits reliable: use swept movement (trace from previous to current position each tick against enemy/player object types) so fast projectiles and headless large-delta frames can't tunnel; keep overlap as secondary. Verify enemy pawns' collision object type/responses actually match the trace channel.
3. Add log marker `EMBER: ProjectileHit` on every resolved hit; the M2 smoke MUST show kills happening via projectile hits only (add a debug assert/log if any other damage path fires during smoke).
This fix is required because Split/Pierce/Ricochet below are meaningless without real per-projectile hit resolution.

## Scope: level-up → pause → 3 upgrade cards → apply → reset at run end

### Data
- `URunUpgradeData : UPrimaryDataAsset` — Id (FName), Title, Description, Rarity (Common/Rare/Epic; weights 60/30/10), MaxStacks, EffectType enum + FloatParams, SoftObjectPtr<UTexture2D> Icon (may be null now; generated icons land in `/Game/Emberbound/Generated/UpgradeIcons/T_<kebab-id>` at M5 — resolve by convention at runtime if the soft ref is unset, fallback = rarity-tinted swatch).
- Editor python `Tools/EditorScripts/create_upgrade_data.py` creating all 15 assets under `/Game/Emberbound/Data/Upgrades` with BALANCE.md values: split-shot, piercing-shot, ricochet-shot, rapid-fire, heavy-arrow, fire-arrow, poison-arrow, frost-arrow, bone-shield, healing-wisp, pickup-magnet, crit-focus, companion-slime, soul-flame-trail, gold-rush. Print `CONTENT_OK`.

### Systems
- `URunUpgradeManager` (world subsystem) — offer roll (3 distinct, rarity-weighted, respect MaxStacks), Apply(id) routes to gameplay: AutoAttack fields (split count, pierce, ricochet, atk-speed/damage multipliers, crit), status-effect payloads on projectiles (burn 4dps×3s / poison 2dps×6s stacking / frost −30% move 2s — extend HealthComponent with an FStatusEffect array ticked at 4 Hz), Bone Shield orbiter (pooled actor circling player, 8 dmg on touch), Healing Wisp (+2 HP/3 s timer), Pickup Magnet (+50% radius), Companion Slime (friendly pawn, acid blob 6 dmg @1.2 s, reuses enemy visuals green-tinted), Soul Flame Trail (pooled 5 dmg/s hazard patches dropped on a 0.3 s move timer, 2 s life), Gold Rush (+25% gold multiplier in run tally). Everything resets/despawns on run end.
- Level-up flow: XPComponent OnLevelUp → `SetPaused(true)` (widgets must keep working: set widget tick while paused) → UpgradeChoiceWidget (C++-built tree like the HUD: 3 large cards, title/desc/rarity color/icon, clickable + keys 1/2/3) → apply → unpause. Queue multiple pending level-ups.
- Debug/verification hooks: console vars `Ember.AutoPickUpgrades` (auto-select random card 0.5 s after show — needed for headless smoke) and exec `Ember.GrantXP <n>`.

### Verification (run + report)
1. UBT green. 2. Content script `CONTENT_OK` + asset-exists check for all 15. 3. Smoke `-game` run with `Ember.AutoPickUpgrades 1`, grant XP periodically via debug auto-grant (dev-only flag): log must show ≥3 `EMBER: LevelUp` and `EMBER: UpgradeApplied <id> stack=<n>` including at least one stacked upgrade, no crashes, combat still functional after (ProjectileFired continues). 4. `Docs/orchestration/M2_report.md` + TEST_REPORT.md row. 5. Commit `M2: run upgrade system`. Print `M2_DONE` when finished.
Out of scope: boss (M3), camp (M4), real icon art (M5).
