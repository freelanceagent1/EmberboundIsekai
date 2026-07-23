# CONTENT_PROPOSAL.md

Proposed (non-canon) content authored for release-scope UI coverage. Names, identities, and ability *concepts* only — every number is a `{token}` pending BALANCE.md revision. Canonical entities are marked ✓.

## Biomes (3)
1. **Ember Meadows** ✓(implied by slice) — scorched grasslands; hazard: ember squalls. Boss: Tutorial Dragon ✓.
2. **Gloomroot Hollow** — sunken forest, Soul Ash economy; hazard: gloom pools ({hazard_dps}). Boss: **The Hollow Sovereign** (GiantSpirit sprite) — lantern volleys, arena-flooding Gloom Tide, wisp summons.
3. **Cinder Wastes** — volcanic Rift crater; hazard: cinderstorm. Boss: **Grudgekeeper Vorn** (DemonCyclop sprite) — targeted meteors, chain sweeps, persistent Cinder Roar enrage.

## Enemy families (12 common: 5 canonical ✓ + 7 proposed)
Canonical: Green Slime, Red Slime, Goblin Stabber, Goblin Archer, Bone Rat.
Proposed: Moss Kappa, Thorn Larva (Meadows); Gloom Bat, Rattle Skull, Frost Skull, Shade Wisp, Creep Shroom (Hollow); Cinder Flam, Ash Flam, Dune Lizard, Sand Snake, Rift Eye, Ember Spider (Wastes). *(13 listed; Rattle/Frost Skull share one family — the Skull family — keeping the family count at 12.)*

## Elites (6)
Elder Kappa, Alpha Mole; Grave Cyclops, Gloom Matron; Dune Basilisk, Rift Warden. Each has one telegraphed signature and doubles the room cache (canonical elite ×2 rule ✓).

## Tameable roster (9)
Canonical ✓: Slime, Bone Rat, Skeleton Archer (roles/rates preserved verbatim).
Proposed: Moss Kappa, Thorn Larva, Gloom Bat, Shade Wisp, Cinder Flam, Dune Lizard — each with Worker/Companion/Defender suitability and a camp passive (e.g. Cinder Flam "Hearthborn": keeps the Campfire lit, +offline rate). Duplicates convert to shards; ascension at {pet_shards} shards, 3 tiers.

## Weapons (5 bow families — BALANCE.md authorizes bows only)
1. Ashwood Recurve (common, baseline)
2. Emberfang Longbow (rare, Burn on hit)
3. Gloomstring (rare, pierce + backstab bonus)
4. Rift Splitter (epic, twin arrows)
5. Sovereign's Lament (legendary greatbow, charged AoE shots)
Enhance to +5 with Gold + Soul Ash ({weapon_upgrade_cost}); dismantle returns {dismantle_pct}%. No evolution/reforge in v1 (deferred decision).

## Artifacts (20, 4 slots: Trinket / Quiver / Charm / Relic)
Full list in `data/proposed.js`. Tone leans dark-comedy isekai (Office Drone Badge, Lucky Truck Charm, Genre-Savvy Script). One presentation-only set: **Sovereign's Regalia** (2-piece bonus).

## Permanent skill tree (3 branches × 10 nodes)
- **Marksman** (ember) — damage/crit; capstone: Rift-Pierced Arrow.
- **Survivor** (green) — HP/mitigation; capstone: Ember Heart.
- **Ashbinder** (violet) — economy/boons/pets; capstone: Rift Bargain.
Currency: Soul Ash ✓; cost {skill_cost} by tier; full-refund respec at Campfire Core ({respec_cost}).

## Open decisions requiring approval before Unreal work
1. Real numbers for every `{token}` (BALANCE.md revision).
2. Whether biome-2/3 bosses drop Embers at the Tutorial Dragon's 30% rate.
3. Skull family: one family with two variants vs two families (affects bestiary count).
4. Weapon evolution/reforge inclusion.
5. Pet ascension visual treatment (palette-swap variants exist in Ninja Adventure pack).
