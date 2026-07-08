# Android Packaging Notes

## Project Settings
- Package name: configured in project settings for Emberbound Isekai.
- Orientation: landscape.
- Touch: native `UVirtualJoystickWidget` is shown on touch-capable platforms or when `Ember.ShowJoystick 1` / saved debug joystick setting is enabled.
- `bAlwaysShowTouchInterface`: leave Unreal's default touch interface disabled for this project path; M8 uses the custom UMG joystick instead so desktop smoke can validate the same widget.

## Required Local Setup
- Install Android Studio and the Unreal-supported Android SDK, NDK, JDK, and platform tools through the UE Android setup flow.
- In UE 5.8, verify Android SDK/NDK/JDK paths under Project Settings > Platforms > Android SDK.
- Accept Android SDK licenses before packaging.

## Texture And Audio Budget
- Current generated art is intentionally small stylized placeholder content.
- Keep mobile textures compressed and avoid replacing the generated placeholders with large uncompressed imports.
- SFX are mono WAV placeholders imported under `/Game/Emberbound/Generated/SFX`; replace with compressed platform settings before a store build.

## Package Steps
1. Run the Win64 editor build first with `-MaxParallelActions=10`.
2. Generate/import M8 title and SFX assets.
3. Open the project in UE 5.8.
4. Confirm Android package name, landscape orientation, and signing profile.
5. Use Platforms > Android > Package Project.
6. Install the APK/AAB on a landscape Android device and verify camp, expedition, joystick, run result, save/reload, and audio.

## Untested On This Machine
- Android SDK/NDK is not verified in the M8 environment.
- No physical Android device smoke was run.
- Store signing, Play Asset Delivery, device thermal behavior, and final compression settings remain untested.
