# WORK ORDER M15 — Make it feel like a real game: menu, onboarding, tooltips, clarity, UI theme, composition

Context (user feedback): "not even close to a playable game… no tooltips, not clear game mechanics." The systems work but the game does not present or explain itself. Fix that. A parallel art pass is regenerating ALL sprites/icons under one cohesive flat-2D style; you will RE-IMPORT that art as your final step.

Same hard rules (UBT green `-MaxParallelActions=10`, one editor process at a time, never kill foreign processes — only your own PIDs, original content). Reuse existing C++-built-widget patterns.

## Scope
1. **Title / main menu screen** (new first screen on boot): full-screen `T_MainSplash` key art, game title, big **PLAY** button (New Game / Continue based on save), Settings button (volume, joystick toggle), Quit. Boot flow: MainMenu → Camp. Add a `L_MainMenu` map (duplicate-template pattern) or a boot widget over Camp. Log `EMBER: MainMenuShown`.
2. **First-run onboarding** (shown once, saved flag `bSeenTutorial`): a short sequence of dismissable hint cards on first Camp + first Expedition: "Drag the left side of the screen to move — you attack automatically", "Tap EXPEDITION to enter the dungeon", "Clear each room; on level-up pick 1 of 3 upgrades", "Reach Room 7 and defeat the Tutorial Dragon", "Back at camp, spend resources to grow stronger". Debug: `Ember.ResetTutorial`. Log `EMBER: Onboarding step=<n>`.
3. **Tooltips / info everywhere**: every Camp building and HUD button gets a short description on tap/hover (Campfire Core: "Grows the safe zone & offline storage", Grim Forge: "+15% bow damage per level", Monster Pen: "Assign tamed monsters as workers/companions/defenders", Watchtower: "Start a Camp Defense wave", resources: name + what they're for). Upgrade cards already show title/desc — ensure rarity color + one-line effect is legible. Add a small `(?)` help affordance on each Camp panel.
4. **Objective clarity in Expedition**: persistent objective line ("Room X / 7 — clear all enemies"), a clear "BOSS — Tutorial Dragon Lv.3" banner on room 7, and when a room is cleared show "Room cleared! → head to the portal" with the portal pinged/highlighted. Enemy/boss health bars readable.
5. **UI theme pass** — replace the plain gray buttons/labels with a consistent parchment+ember theme: warm cream/parchment panels with an ember-orange accent and bone outlines, rounded, larger touch targets, consistent font sizes and spacing. Apply across Camp HUD, panels, upgrade cards, result screen, offline popup, defense UI. Keep it readable on a phone (landscape, safe-area).
6. **Camp composition fix**: the camp currently shows a hard-edged floating "dirt platform" square on the grass and tiny buildings in a big empty field. Make the camp read intentionally: either remove the dirt platform and place buildings on the grass, or make it a soft round clearing; tighten the camp orthographic `OrthoWidth` so buildings are large and clearly arranged; label each building with a small nameplate. Keep it 2D/orthographic (M13).
7. **Sprite scale consistency**: ensure enemies/player/buildings are at consistent, readable phone scale (no tiny sprites lost in a big field).

## Final steps (do LAST, after the art pass is done — the orchestrator will confirm)
8. **Re-import** the regenerated cohesive art: `set EMBER_FORCE_REIMPORT=1` and run the import script so all `/Game/Emberbound/Generated/*` textures match the new `GeneratedAssets/processed/`. Rebuild materials if needed.
9. **Re-verify**: UBT green (both targets); autopilot still logs `AUTOPILOT COMPLETE`; capture fresh `Docs/validation/shot_menu.png`, `shot_camp.png`, `shot_expedition.png`, `shot_boss.png` and confirm by inspecting them that the game reads as a cohesive, themed, self-explaining 2D phone game. **Re-package** the standalone build.

## Report
`Docs/orchestration/M15_report.md` + TEST_REPORT rows. Commit `M15: menu, onboarding, tooltips, UI theme, camp composition`. Print `M15_DONE`.
