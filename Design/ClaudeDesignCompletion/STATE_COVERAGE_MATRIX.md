# STATE_COVERAGE_MATRIX.md

Reachability: **N** = normal navigation, **R** = review jump rail shortcut (all N states are reachable without R).

## Menu & app
| State | Reach | Notes |
|---|---|---|
| Menu, no save / with save (Continue enabled) | N | Continue disabled until save exists |
| New Game confirm | N | resets model, replays first-launch |
| Settings / Credits from menu | N | overlay returns to exact opener context |
| Settings / Credits from Camp | N | same overlay, back label reflects context |
| Quit handoff (truthful platform state) | N | no fake quit |

## Camp
| State | Reach |
|---|---|
| Camp overview: buildings, defense, worker production, portal | N |
| Start Defense enabled (defender assigned) / disabled + reason | N |
| Offline reward dialog: pending → claim once → not re-claimable | N (Continue → Camp) |

## Character
Empty / partially equipped / complete loadout message; derived-stat explanations (hover ⓘ); equipped weapon + 4 artifact slots (empty state per slot, click-through); active pet; skill summary per branch; temporary run boons section only while a run is active. All N.

## Skill tree (30 nodes, 3 branches)
| Node state | Shown as |
|---|---|
| Undiscovered/locked | "???" dimmed |
| Prerequisite-locked | "needs prerequisite" |
| Discovered | tier ≤2, visible, unaffordable styling |
| Affordable / Unaffordable | branch-color edge / "not enough Soul Ash" |
| Purchased / Maxed capstone | green ✓ / gold ★ |
Purchase confirm (button + Enter), success (toast + inline msg), failure (disabled + reason). Respec explanation + confirm dialog with refund math. Keyboard arrows/Enter node navigation (gamepad-sim). All N.

## Arsenal (5 bow families)
Locked silhouette, owned, equipped, favorite, selected; rarity filter; detail + compare vs equipped; equip; enhance with cost + insufficient-material state + max level; dismantle confirm with exact returned-material preview; equipped weapons can't be dismantled. Inventory-full handled at loot reveal. All N.

## Artifacts (20, 4 slots)
Slot chips (empty/filled), slot filters, locked/owned/equipped/selected, detail + replace-vs-current compare, equip/unequip/replace, empower (insufficient state, max), dismantle confirm with returns, duplicate note, set bonus line (Sovereign's Regalia). All N.

## Pets (9 roster)
Undiscovered silhouette → discovered grayscale + shard count → summonable (tame CTA) → unlocked (level/XP/ascension stars). Feed (cost, insufficient, max), Ascend preview + confirm + insufficient, role rows per pet (unsuited, assign, unassign, slot-locked by Monster Pen, conflict resolution swap dialog), duplicate/shard conversion explanation. All N.

## World (3 biomes)
Locked / NEW / available / cleared / mastered; difficulty radio; expected rewards + canonical room plan; enemy/elite/boss/tameable preview (silhouette until discovered, click → bestiary); unlock requirements; pre-run loadout review with warnings. All N.

## Bestiary (12 common + 6 elite + 3 boss)
Filters (All/Common/Elite/Boss/Tameable); unknown silhouette; discovered dossier: abilities, telegraphs, drops, habitat, resistances, tameability; boss dossier adds phases + defeat status. All N.

## Expedition run
Room 1–5 → boon pick (persists + stacks, "×1 → ×2" shown) → elite → boss → loot reveal sequence (currency, shards, weapon w/ keep-compare-dismantle, artifact, inventory-full resolution) → results (victory 100% / retreat-defeat 60% banking, permanent vs temporary build summary) → Return to Camp / retry. Retreat confirm from HUD. All N.

## Camp Buildings
Per building: level, next-level change, per-resource affordability, insufficient state, max state, Monster Pen prerequisite lock (Campfire Core Lv2, canonical), upgrade confirm/success/save feedback; upgrades mutate wallet + model and persist. All N.

## Review-only (R)
Jump rail, aspect frames, a11y toggles, Reset prototype data, demo Results state. Review navigation does not reset persistent flags; only "Reset prototype data" does.
