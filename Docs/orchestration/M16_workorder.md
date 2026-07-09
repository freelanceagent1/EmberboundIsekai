# WORK ORDER M16 — Camp layout fix + readability polish

Context: after M14 (cohesive art) + M15 (menu/onboarding/tooltips), the menu/expedition/boss look great, but the **Camp screen is broken**: the camp buildings are spawned bunched up in the center **overlapping each other and the player**, their nameplates collide, and **one building's nameplate literally reads "Text"** (a display name / TextRenderComponent default not set). Also the meadow ground was a harsh saturated green and top HUD text has low contrast against it. See `Docs/validation/shot_camp.png`.

Same hard rules (UBT green `-MaxParallelActions=10`, one editor process at a time, never kill foreign processes — only your own PIDs, original content). Keep the M13 orthographic 2D look and the M15 menu/onboarding/tooltips/theme.

## Fix
1. **Spread the camp buildings out into a clear, readable arrangement** so none overlap each other or the player at the current camp `OrthoWidth`. E.g. place Campfire Core in the center, and Grim Forge / Monster Pen / Watchtower / Expedition Portal evenly around it in a ring (or a tidy 2-row layout) with clear spacing. The player spawns in a clear spot (front/center). Building sprites sized so the whole camp reads without overlap. Verify by screenshot that every building is separated and legible.
2. **Fix the "Text" nameplate**: every building's nameplate must show its real display name (Campfire Core, Grim Forge, Monster Pen, Watchtower, Expedition Portal). The one showing "Text" is a `UTextRenderComponent` whose text was never set (likely the campfire/core actor) — set it from the building's DisplayName. Position nameplates ABOVE each building without overlapping neighbors.
3. **Readability**: the meadow ground texture was re-toned (less saturated) — re-import it. Add a subtle dark translucent strip/panel behind the top-left HUD text (goal text, resources, tooltip line) so it's readable against the bright field. Ensure objective/goal/nameplate text has enough contrast.
4. Keep sprite scale consistent and mobile-readable.

## Verify
1. UBT green (both targets). 2. Re-import generated art (`EMBER_FORCE_REIMPORT=1`) so the re-toned meadow + any changes apply. 3. Autopilot still logs `AUTOPILOT COMPLETE`. 4. Re-capture `Docs/validation/shot_camp.png` (and menu/expedition/boss) and CONFIRM by inspecting: camp buildings are spread out, separated, each with a correct nameplate (no "Text"), player clear, HUD text readable. 5. Re-package the standalone build. 6. `Docs/orchestration/M16_report.md` + TEST_REPORT rows. Commit `M16: camp layout + readability polish`. Print `M16_DONE`.
