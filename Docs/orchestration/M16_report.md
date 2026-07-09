# M16 Report - Camp layout + readability polish

## Changes
- Reworked the Camp spawn layout into a clear five-building arrangement at the current 980 orthographic width: Campfire Core central, Grim Forge/Watchtower above, Expedition Portal/Monster Pen below, and the player moved to a clear front-center position.
- Fixed nameplate setup at the source. Campfire Core now uses the same nameplate path as the other buildings, every building resolves a real display name, and `Expedition Portal` no longer falls back to a shortened label.
- Removed the hidden widget nameplate path that was still trying to instantiate raw `UUserWidget`; final camp capture log has no PIE widget errors.
- Re-imported generated art with `EMBER_FORCE_REIMPORT=1`, including the re-toned meadow grass.
- Added a subtle translucent dark HUD strip and switched camp HUD text/resource/help copy to light text with shadow for contrast.
- Added `-EmberSkipOnboarding` as a validation-only command-line flag so screenshots can capture the camp layout without the first-run hint card covering labels.
- Added `Emberbound.M16.CampPresentationDefaults` automation coverage for real building names and layout spacing.

## Validation
- UBT editor target: `EmberboundIsekaiEditor Win64 Development` succeeded with `-MaxParallelActions=10`.
- UBT game target: `EmberboundIsekai Win64 Development` succeeded with `-MaxParallelActions=10`.
- Generated art re-import: `Docs/validation/M16_reimport.log` ended with `IMPORT_OK n=62` and `UPGRADE_ICONS_ASSIGNED n=15`.
- Automation: `Docs/validation/M16_automation.log` ran `Emberbound.M16.CampPresentationDefaults` with `Result={Success}` and exit code 0.
- Autopilot: `Docs/validation/M16_autopilot.log` logged the ordered loop through `AUTOPILOT COMPLETE` with no `Fatal` or `Error: [C++]` grep hits.
- Editor screenshots refreshed and inspected:
  - `Docs/validation/shot_camp.png` - separated buildings, correct labels for all five, no `Text` placeholder, player clear, HUD readable.
  - `Docs/validation/shot_menu.png` - M15 menu/key art/theme still intact.
  - `Docs/validation/shot_expedition.png` - objective/HUD and expedition sprites still intact.
  - `Docs/validation/shot_boss.png` - boss banner, health bar, and Tutorial Dragon still intact.
- Package: RunUAT `BuildCookRun` Win64 Development succeeded with `-ubtargs=-MaxParallelActions=10`; output is under `Packaged/Windows`.
- Packaged smoke:
  - `Docs/validation/packaged_autopilot.log` logged `AUTOPILOT COMPLETE`.
  - `Docs/validation/packaged_camp.png` was captured from the top-level packaged exe and matches the final camp layout/readability pass.

## Notes
- The packaged autopilot helper copied the log path incorrectly for this package layout, so I replaced that artifact with the real packaged log from `Packaged/Windows/EmberboundIsekai/Saved/Logs/EmberboundIsekai.log` and copied the targeted packaged camp screenshot manually.
- The untracked `Docs/validation/01_camp_t81832s.png` was already present before M16 work and was left untouched.
