# M11 Report - Standalone Windows package

## Package output
- Packaged bootstrap exe: `E:\mobilegamer\EmberboundIsekai\Packaged\Windows\EmberboundIsekai.exe`
- Packaged game exe: `E:\mobilegamer\EmberboundIsekai\Packaged\Windows\EmberboundIsekai\Binaries\Win64\EmberboundIsekai.exe`
- Configuration: Win64 Development, cooked, staged, pak/iostore archive.
- Final UAT log: `Docs/orchestration/codex_M11_package_20260708_131845.log`
- Final UAT result: `BUILD SUCCESSFUL`, `AutomationTool exiting with ExitCode=0 (Success)`.

## Packaging config and fixes
- Added `MapsToCook` entries for `/Game/Emberbound/Maps/Camp/L_Camp` and `/Game/Emberbound/Maps/Expedition/L_Expedition`.
- Added always-cook directories for `/Game/Emberbound/Generated`, `/Game/Emberbound/Data`, and `/Game/Emberbound/Materials/Generated`.
- Added Asset Manager scan rules for Building, Enemy, Monster, EmberTitle, and RunUpgrade primary data assets.
- Fixed packaged runtime data discovery by adding deterministic cooked-build fallback loads for known Emberbound data assets and run upgrades when the cooked asset registry query is empty.
- Added the `EMBER: IconResolved` marker to the camp HUD, since the M11 packaged boot verifies `L_Camp` and the previous marker only existed on the expedition HUD.

## Boot verification
- Command:
  ```text
  E:\mobilegamer\EmberboundIsekai\Packaged\Windows\EmberboundIsekai.exe /Game/Emberbound/Maps/Camp/L_Camp -RenderOffScreen -nosplash -stdout -resx=1280 -resy=720 -ExecCmds="Ember.CaptureAfter 8 showui"
  ```
- Verification log: `Docs/validation/M11_packaged_boot_finalpass.log`
- Stopped only the two packaged `EmberboundIsekai.exe` PIDs launched for this verification.
- Required markers:
  ```text
  EMBER: UpgradeCatalogLoaded count=15
  EMBER: SaveLoaded Gold=0 Wood=0 SoulAsh=0 campfire=0 forge=0 slimeShards=0 owned=0 assignments=0 titles=1 volume=1.00 joystick=0
  EMBER: DataRegistryLoaded Count=30
  Game class is 'CampGameMode'
  EMBER: IconResolved 5/5
  EMBER: CaptureFired file=HighresScreenshot00007.png
  ```
- Screenshot copied to `Docs/validation/packaged_camp.png`.

## Deviations / notes
- The first UAT launch attempt failed before running because `cmd.exe` split the quoted `C:\Program Files\...` RunUAT path. Re-ran using `Start-Process` directly with an argument array.
- A later archive retry saw the packaged inner exe locked by the packaged boot process I had launched. I stopped only that launched packaged game child, then reran to a clean final package.
- Cook emitted non-target SDK warnings for Android/Linux/LinuxArm64/VisionOS while Win64 remained valid; no action required for the requested Windows package.
