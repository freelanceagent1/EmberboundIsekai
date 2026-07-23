# SELF_REVIEW.md

Checklist executed against the finished scaffold (2026-07-16).

1. **Required screens/states in STATE_COVERAGE_MATRIX.md** — ✅ all sections of the brief mapped; every required state row present.
2. **Normal flows reachable without review rail** — ✅ Menu→Continue→Camp→(offline claim)→tabs→World→loadout→run rooms 1-5→boons→elite→boss→loot→results→Camp; retreat path; Settings/Credits return to opener; Quit is a truthful handoff.
3. **All buttons/navigation work** — ✅ every control is a real `<button>` wired to a state mutation or explains its disabled state; verifier pass requested on delivery.
4. **Actions visibly mutate prototype state** — ✅ equip/enhance/dismantle (weapons, artifacts), skill purchase + respec refund, building upgrades (wallet deducts, level saves), pet tame/feed/ascend/assign + conflict swap, boon stacks (×n shown), offline claim (once), defense bounty, run banking (100%/60%). Persisted to localStorage `ebi-expanded-proto-v1` with version key + review-only reset.
5. **No canonical conflicts** — ✅ all repository values transcribed into `data/canonical.js` verbatim (player stats, enemy stats, dragon, boons, building costs/effects, pet rates, titles, offline cap, banking, save slot). Old HTML's conflicting hard-codes not carried over. New numbers are `{tokens}` or flagged `prototypeOnly` samples.
6. **No remote URLs** — ✅ zero external fonts/scripts/images; data modules are local; the only font is the bundled NormalFont.ttf.
7. **Referenced local assets exist** — ✅ all `assets/…` paths in data files correspond to copied files (verifier-checked; no 404s in console).
8. **All third-party assets in ASSET_USAGE_MANIFEST.csv** — ✅ 162 rows incl. reserved reference files.
9. **CC BY attribution** — ✅ every game-icons file carries author (Lorc), license, per-icon credit line; prototype renders glyph placeholders, SVGs bound for final icons (noted as required modification: recolor).
10. **16:9 / 20:9 / 4:3 / portrait** — ✅ review frames for each; 20:9 includes cutout + safe-area padding; grids are auto-fit and reflow at 400px portrait; content areas scroll.
11. **Touch/focus/dialog/motion/contrast/125%** — ✅ ≥48px primaries, `:focus-visible` outline, logical DOM order, dialogs focus-in + Esc + restore, reduced-motion kills sprite/pop animation, high-contrast filter mode, 125% text mode scales root em. State never encoded by color alone (text labels accompany all chips). Tooltips have click/focus equivalents (detail panels repeat hover info).
12. **Baseline/Unreal untouched** — ✅ all writes confined to `Design/ClaudeDesignCompletion/`; source folders only read.

## Known limitations (honest)
- Artifact icons render as Unicode glyph placeholders; the licensed SVGs are copied and mapped but not inlined (final UMG must use them).
- Combat is a deterministic simulation (fixed-seed pseudo-rolls), not gameplay; damage/HP loss per room is sample-only.
- `env(safe-area-inset-*)` is simulated by frame padding since the review frame is not a real cutout device; the CSS hook is documented in the handoff.
- Skull family counted as one family with two variants to hold the 12-family envelope (flagged as open decision #3).

**Result: a release-complete UI/UX scaffold and implementation reference — not a finished Unreal game.**
