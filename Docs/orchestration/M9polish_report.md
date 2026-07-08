# M9 Polish Report - Billboard orientation, scale, material, and texture pass

## Before
- `Docs/validation/shot_boss.png` showed the Tutorial Dragon as a small dark box/red speck instead of the orange `T_TutorialDragon` art.
- `shot_expedition.png` and `shot_boss.png` had huge HUD/pickup-looking icons dominating the top of the view, while the player/enemy billboards were edge-on or too small.
- `shot_camp.png` made the camp visuals hard to read because the generated billboards and HUD icons were oversized/misinterpreted; the building sprites also used plane dimensions that treated Z as sprite height.

## Root Cause
- Runtime sprite actors use `/Engine/BasicShapes/Plane`, whose useful sprite dimensions are local X/Y. Several actors were scaling local Z for height, which does not resize the plane surface.
- The shared camera-facing helper copied the camera rotator directly onto a plane whose local normal is +Z, instead of rotating the plane normal toward the gameplay camera.
- HUD image widgets used `SetBrushFromTexture(..., true)`, so 512px source textures reported native size and rendered as oversized viewport icons.

## Changes
- Reworked `EmberGeneratedArt::FaceComponentToPlayerCamera` to align the plane normal to the active player camera using a camera-right/up-stable rotation.
- Changed player, enemy, boss, pickup, camp building, and campfire flame billboard dimensions to use XY plane scale and sensible vertical offsets.
- Added explicit camp building texture mapping for `campfire-core`, `grim-forge`, `monster-pen`, `watchtower`, and `expedition-portal`.
- Constrained camp and expedition HUD icon images to 28x28 so they no longer obscure validation screenshots.
- Applied a low-risk runtime tint to the expedition corrupted-grass floor material instance to reduce saturation enough for characters to read.

## Verification
- Build green:
  - `Build.bat EmberboundIsekaiEditor Win64 Development -Project=E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject -WaitMutex -MaxParallelActions=10`
  - Final result: `Result: Succeeded`
- Final captures used one editor process at a time, real RHI, `-RenderOffScreen`, no `-unattended`, and `Ember.CaptureAfter`:
  - `Docs/validation/shot_camp.png`
  - `Docs/validation/shot_expedition.png`
  - `Docs/validation/shot_boss.png`
- Visual confirmation from final PNG inspection:
  - Camp: all five camp building sprites are upright, distinct, and not replaced by resource icons; HUD icons are small.
  - Expedition: player sprite is upright/readable, enemy/pickup sprites are no longer edge-on/oversized, HUD icons are small.
  - Boss: Tutorial Dragon is upright, orange, and clearly recognizable as the generated dragon texture; player and VFX read against the toned floor.

## Notes
- The first post-build capture attempt used too short a timeout for this machine's editor startup and produced no PNG. The bounded capture was rerun with a longer timeout; only editor PIDs launched by this task were stopped.
