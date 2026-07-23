# Design Polish Review and Product Decisions

Reviewed package: `Game design polish request (1).zip`

SHA-256: `400169ACDC689FCDE66966006BB32D9897F6EFA53CA8210DEB6FF677082821D0`

Reviewed: 2026-07-23

## Verdict

The package is accepted as the authoritative UI/UX implementation reference for the next Unreal polish pass. It is not accepted as evidence that the Unreal implementation is complete, and its proposed release-scale content is not silently promoted to game canon.

The current playable vertical slice remains the shipping baseline. The design package supplies the target information architecture, component language, responsive behavior, and coherent art-family rules used to improve that baseline.

## Decisions

### Adopt now

1. **One coherent gameplay art family.** Ninja Adventure's CC0 pixel assets are the primary family for actors, enemies, pets, bosses, environmental tiles, pickups, and gameplay VFX. Pixel assets use nearest-neighbor sampling and normalized integer scaling.
2. **No mixed rendering language inside gameplay.** Existing AI art may remain for splash/key art, camp illustration, and selected currency/branding surfaces, but it must not be mixed beside pixel actors in an expedition or defense scene.
3. **Explicit progression labels.** Replace ambiguous `Upgrades` navigation with `Skills`, `Arsenal`, `Artifacts`, `Camp Buildings`, and `Pet Training` where those systems exist. The current vertical slice may hide routes whose systems are not implemented yet.
4. **Reusable mobile UI components.** Adopt the documented currency chips, cards, state badges, rarity treatments, dialogs, tooltips/detail panels, toasts, safe-area shell, and focus treatment.
5. **Accessible interaction rules.** Keep primary touch targets at least 48 CSS-equivalent pixels, provide keyboard/gamepad focus, repeat hover information in focus/tap detail views, avoid color-only state, and support reduced motion, high contrast, and larger text.
6. **Canonical/proposed data separation.** Existing repository balance and IDs remain authoritative. Proposed content stays visibly marked in review tooling and must enter Unreal through reviewed DataAssets/DataTables.
7. **Truthful state and feedback.** Disabled actions explain why; purchases, assignments, claims, upgrades, and saves show immediate state changes and success/failure feedback.

### Keep as the current vertical-slice canon

- Five normal rooms, one elite room, and the Tutorial Dragon boss room.
- Fifteen temporary run boons.
- Slime, Bone Rat, and Skeleton Archer as the initial tameable roster.
- Campfire Core, Grim Forge, Monster Pen, Watchtower, and Expedition Portal.
- Current player, enemy, reward, upgrade, building, title, offline-cap, and save values from `BALANCE.md` and Unreal DataAssets.
- Tutorial Dragon's current canonical Ember reward chance.

### Approve as future content direction, not current scope

- Three biome identities: Ember Meadows, Gloomroot Hollow, and Cinder Wastes.
- Five bow-family identities rather than unrelated weapon classes.
- The expanded bestiary, additional bosses, nine-pet roster, artifacts, and three-branch permanent skill tree.
- World map, bestiary dossier, equipment comparison, loot-resolution, and collection-management screens.

These features require individual gameplay specifications, balance passes, save-version migration, and acceptance tests before implementation.

### Deferred product decisions

1. **Weapon evolution/reforge:** deferred. First implementation should support collection, equip, compare, and enhance only.
2. **Pet ascension:** deferred. Shards, unlock, assignment, companion/worker/defender roles, and simple training come first.
3. **Artifact sets:** deferred until the base artifact equip/replace loop is proven.
4. **Skull variants:** one visual family for content accounting, separate enemy DataAssets when behavior differs.
5. **New economy values:** unresolved tokens remain tokens. Do not copy prototype-only numbers into `BALANCE.md`.
6. **Additional boss Ember rates:** determine from progression simulations after biome reward budgets exist.

### Rejected

- Implementing all proposed release-scale content in one pass.
- Treating the HTML combat simulation as gameplay validation.
- Importing all staged third-party source archives into Unreal or Git history.
- Using Unicode placeholders as final item/artifact icons.
- Mixing differently scaled pixel packs or AI billboards within one gameplay scene.
- Claiming the design package itself fixes the current Unreal presentation.

## Integration order

1. Build the shared UMG theme and navigation shell from the documented tokens.
2. Restyle the existing Camp, Expedition HUD, boon selection, results, offline reward, and defense panels without changing their gameplay contracts.
3. Convert expedition/defense actors and tiles to the approved pixel family as a complete scene-level swap.
4. Run the complete Camp → Expedition → Dragon → Camp → upgrade/save loop and defense loop.
5. Only then add permanent Skills, Arsenal, Artifacts, new biomes, and expanded collections as separately tested milestones.

## Review evidence

- JavaScript syntax checks passed for `support.js`, `data/canonical.js`, and `data/proposed.js`.
- All 76 literal local asset references found in HTML/JS/Markdown resolve.
- The asset usage manifest contains 161 populated attribution rows.
- The curated completion bundle contains 172 files and is approximately 8.3 MB.
- The package explicitly and correctly states that Unreal integration is not complete.

## Source of truth

- Working game and balance: Unreal C++, DataAssets, `BALANCE.md`, and `TECHNICAL_DESIGN.md`.
- UI/UX target: `Design/ClaudeDesignCompletion/`.
- Future content concepts: `Design/ClaudeDesignCompletion/CONTENT_PROPOSAL.md`.
- License and attribution requirements: `Design/ClaudeDesignCompletion/ASSET_PROVENANCE.md` and `ASSET_USAGE_MANIFEST.csv`.
