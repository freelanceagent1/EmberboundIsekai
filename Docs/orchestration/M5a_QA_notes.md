# M5a QA notes (orchestrator spot-check, 2026-07-07)

Reviewed samples from GeneratedAssets/processed/:

| Asset | Verdict | Issue |
|---|---|---|
| keyart/main-splash_1536x1024 | ✅ excellent | none — use for main menu as-is |
| characters/tutorial-dragon_1024 | ✅ excellent | clean alpha, perfect style |
| characters/rift-archer-full_1024 | ⚠️ usable | large residual white/grey background block behind legs survived keying |
| ui-icons/gold_512 | ❌ needs fix | model rendered WHITE background (not magenta) + full-frame coin scatter; keyer (magenta-hue-based) silently passed it through opaque |

## Required fix pass (fold into M5 import milestone, before UE import)
1. Extend post-processing: detect near-solid white/grey corner backgrounds and key them too (corner flood-fill, tolerance), not just magenta-hue.
2. Audit all 28 ui-icons/upgrade-icons for opaque backgrounds; re-key or regenerate failures with a stronger "single centered object on flat magenta background" prompt and/or icon-frame composition.
3. Regenerate rift-archer-full with simplified prompt if re-keying can't clean it.
4. Icons that are scatter-compositions (like gold) are acceptable if fully keyed; prefer single-object icons on regeneration.
