# M15 Report - Menu, onboarding, clarity, UI theme, camp composition

## Implemented
- Added a boot main menu over Camp using `T_MainSplash`: title, PLAY, SETTINGS, QUIT, volume/joystick settings, and `EMBER: MainMenuShown`.
- Added first-run onboarding hint cards for Camp and Expedition, saved by `bSeenTutorial`, with `Ember.ResetTutorial` and `EMBER: Onboarding step=<n>` logging.
- Added camp/expedition clarity: camp goal text, resource/button/tooltips, building panel help affordances, persistent expedition objective text, room-cleared messaging, portal highlight/pulse, and room-7 `BOSS - Tutorial Dragon Lv.3` banner.
- Added a shared parchment + ember UI style and applied it across camp HUD, building panels, upgrade choices, run result, offline reward, defense status, and defense result widgets.
- Reworked camp composition: removed the floating dirt-platform square from runtime camp presentation, tightened the orthographic camera, clustered buildings, enlarged building sprites, and added visible building nameplates.
- Kept sprite scale consistent for camp buildings, expedition portal, player/enemy/boss presentation, and mobile HUD readability.
- Patched autopilot to bypass the interactive main-menu gate and explicitly start camp presentation when validating the automated loop.

## Generated art final pass
- Re-imported generated cohesive art with `EMBER_FORCE_REIMPORT=1`.
- Import output: `IMPORT_OK n=62`; `UPGRADE_ICONS_ASSIGNED n=15`.
- Recaptured validation screenshots after reimport and presentation fixes:
  - `Docs/validation/shot_menu.png`
  - `Docs/validation/shot_camp.png`
  - `Docs/validation/shot_expedition.png`
  - `Docs/validation/shot_boss.png`

## Verification
- UBT editor: `EmberboundIsekaiEditor Win64 Development` succeeded with `-MaxParallelActions=10`.
- UBT runtime: `EmberboundIsekai Win64 Development` succeeded with `-MaxParallelActions=10`.
- Final autopilot: `Tools/Validation/run_bounded.ps1 ... -Tag M15_autopilot_final2 ...` returned `until_hit=True` for `AUTOPILOT COMPLETE`.
- Final package: RunUAT `BuildCookRun` Win64 Development succeeded with `-ubtargs=-MaxParallelActions=10`; output is under `Packaged/Windows`.
- Screenshot review:
  - Menu reads as a titled game entry screen with PLAY/SETTINGS/QUIT over generated key art.
  - Camp reads as a 2D phone camp screen with goal text, resources, help text, hint card, clustered buildings, no dirt-platform square, and building labels.
  - Expedition reads as an objective-driven room run with onboarding, resource/HUD context, and room-cleared instruction.
  - Boss reads as room 7 with a clear Tutorial Dragon banner/objective and boss sprite.

## Deviations / notes
- Used a boot widget over Camp rather than adding a separate `L_MainMenu` map; this preserved the existing travel/default-map flow while satisfying the first-screen menu requirement.
- HighResShot logs `CaptureFired` before the PNG is fully flushed, so final captures wait after the marker before stopping only the editor PID launched for the capture.
- Non-Windows SDK validation warnings during packaging are unrelated to the Win64 target; Win64 package completed with `BUILD SUCCESSFUL`.
