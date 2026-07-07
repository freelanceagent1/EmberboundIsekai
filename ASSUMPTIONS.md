# ASSUMPTIONS.md — Emberbound Isekai

All practical assumptions made during autonomous development. Updated per milestone.

## Environment (verified 2026-07-07, Milestone 0)

| Item | Status | Decision |
|---|---|---|
| Unreal Engine | UE **5.8** at `C:\Program Files\Epic Games\UE_5.8` (complete, registry-registered). A `UE_5.7` folder exists on E: but has **no editor binary** (incomplete install). | Use UE 5.8. |
| C++ toolchain | VS Community ("18") + VS2022 Build Tools with VC x64, dotnet 10. | Build via UnrealBuildTool `Build.bat`. |
| Unreal MCP | `unreal-mcp` registered in global Claude MCP config (`http://127.0.0.1:8000/mcp`) but **no MCP plugin or server exists anywhere on disk** and port 8000 is closed. | **Fallback (per spec "implement fallback versions"):** all editor automation runs through headless `UnrealEditor-Cmd.exe` Python editor scripting (`PythonScriptPlugin` enabled in the project) + UBT for compiles. This is functionally equivalent, deterministic, and honors "one editor operation at a time" by design (single headless process). If the user later installs the MCP plugin/server, it can be pointed at this project without changes. |
| ComfyUI | Install at `E:\ComfyUI` (venv + `ComfyUI_source` + models dir). Server was **down**; launched by orchestrator on `127.0.0.1:8188`. | Use this install. |
| ComfyUI models | **No text-to-image base checkpoint present** — only face-restore (GFPGAN/CodeFormer/GPEN) and face-swap LoRAs. | At Milestone 5: download ONE small open-license checkpoint (SDXL-Turbo or SD1.5-class, documented in ASSET_MANIFEST.md) since generated assets are a hard deliverable; if download is refused/fails, fall back to stylized procedural placeholder assets and document. GPU is RTX 5070 Ti 16GB — SDXL-class is comfortable. |
| Workers | `codex` CLI 0.142.5 (logged in) = primary code author. Claude subagents (haiku/sonnet) = docs, data authoring, review, UE Python scripts. Orchestrator (this agent) only scaffolds, delegates, compiles, verifies. | Per user instruction. |

## Product decisions

1. **Landscape orientation.** Chosen because top-down auto-shooter combat needs horizontal sightlines to enemies on both flanks, the right thumb rests naturally on the screen's lower-left for the virtual joystick while UI (HP/XP/room counter) sits along the top edge, and UE5 PIE/desktop testing maps 1:1 to a 16:9 landscape viewport. Documented per spec.
2. **Project location:** `E:\mobilegamer\EmberboundIsekai` (working dir given by user; E: has ~150GB free).
3. **Player character** is named **"Rift Archer"** (original). All names, enemies, items are original; no references to any existing game IP.
4. **3D representation:** simple primitive/engine meshes with generated textures + billboard sprites for characters where sensible (spec allows this when full model generation is unavailable).
5. **Tutorial Dragon, Level 3** is literally level 3 as a joke ("only a level-3 dragon") — boss difficulty tuned to be beatable on run 2–3.
6. **Truck-Kun Survivor title** ships **disabled by default** (debug flag) per spec's own suggestion, to cut scope.
7. **Genre-Savvy Idiot title** grants **+5% XP** (no reroll UI in MVP).
8. **Save format:** single `USaveGame` slot `"EmberboundSave"`, autosaved at safe points (camp entry, run end, upgrade purchase).
9. **Offline income** capped at `4h × campfire level` of production; simulated-time debug button included.
10. **PIE validation** is performed via the editor in `-game` mode runs + automation smoke tests headlessly; final human PIE confirmation is listed in TEST_REPORT.md as a manual checklist item.
11. **Git**: project is initialized as a local git repo for milestone checkpoints (no remote).
12. **Enhanced Input** is available (built-in plugin) and used; touch joystick uses UE's virtual joystick + custom UMG fallback.
13. **Mobile targets:** project settings target Mobile/Scalable, landscape, Android package `com.emberbound.isekai`. Actual device packaging (SDK/NDK) is documented but not executed — no Android SDK verified on this machine (packaging notes in README).

## Open risks

- UE 5.8 API drift vs worker-model knowledge: mitigated by compile-gate loop (workers get compiler errors back until green).
- ComfyUI checkpoint download size/time; mitigated by choosing a turbo-class small model.
- First cook/PIE boot of a fresh project takes several minutes of shader compilation.
