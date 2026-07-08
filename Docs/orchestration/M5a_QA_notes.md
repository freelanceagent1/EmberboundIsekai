# M5a QA notes (orchestrator spot-check, 2026-07-07)

Reviewed samples from GeneratedAssets/processed/:

| Asset | Verdict | Issue |
|---|---|---|
| keyart/main-splash_1536x1024 | ✅ excellent | none — use for main menu as-is |
| characters/tutorial-dragon_1024 | ✅ excellent | clean alpha, perfect style |
| characters/rift-archer-full_1024 | ⚠️ usable | large residual white/grey background block behind legs survived keying |
| ui-icons/gold_512 | ❌ needs fix | model rendered WHITE background (not magenta) + full-frame coin scatter; keyer (magenta-hue-based) silently passed it through opaque |

## Resolution (2026-07-07, orchestrator)
- Full audit (subagent): 15 broken assets found, 13 fixed via smarter keyer + regeneration (see M5a_report.md "QA pass").
- gold: first regen introduced a Bitcoin-glyph coin (rejected — third-party mark), second regen (seed 424242) had an un-keyed pink cylinder (rejected). Final: seed 770320 + blank-coin prompt + anti-glyph negatives → clean unmarked coins. ACCEPTED.
- rift-archer-full: seed lottery made things worse; reverted to known-good seed 886923 and removed the residual grey band with a bounded desaturation-mask erase + speckle cleanup (scratchpad band_surgery.py, logic worth folding into generate_assets.py later). ACCEPTED.
- Remaining numeric-audit "failures": hit-spark 256/512 only — verified legitimate edge-to-edge ray composition, background fully transparent. No action.

## Original fix-pass requirements (all addressed)
1. Extend post-processing: detect near-solid white/grey corner backgrounds and key them too (corner flood-fill, tolerance), not just magenta-hue.
2. Audit all 28 ui-icons/upgrade-icons for opaque backgrounds; re-key or regenerate failures with a stronger "single centered object on flat magenta background" prompt and/or icon-frame composition.
3. Regenerate rift-archer-full with simplified prompt if re-keying can't clean it.
4. Icons that are scatter-compositions (like gold) are acceptable if fully keyed; prefer single-object icons on regeneration.
