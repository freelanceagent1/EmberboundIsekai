# WORK ORDER M10 — Continuous end-to-end autopilot + damage numbers

Context: the vertical slice is feature-complete and each stage has been verified in isolation. Two gaps remain for a "fully functional, end-to-end tested" delivery:
(1) **Floating damage numbers** — the one combat-feedback item named in the spec (pooled damage text) that is not yet implemented.
(2) **A continuous single-session run of the WHOLE loop across real map travel** — so far every stage was launched separately; the actual Camp↔Expedition travel + state handoff + banking + save chain has never been exercised end-to-end in one process.

Same hard rules (UBT green `-MaxParallelActions=10`, one editor process at a time, never kill foreign processes — only your own PIDs, original content, pooling/perf).

## Task A — Pooled floating damage numbers
- Add a pooled damage-text system (reuse `UObjectPoolSubsystem`): when `UHealthComponent::ApplyDamage` resolves, spawn a small world-space or screen-projected UMG/`UWidgetComponent` (or a Niagara/CPU sprite text) showing the damage amount, rising + fading over ~0.6 s, then returned to the pool. Crits larger/colored. Cap concurrent texts (e.g. 32). No per-tick allocations; pool acquire/release only.
- Hook player→enemy and enemy→player (and companion/defender) damage. Log `EMBER: DamageText n=<count>` periodically (throttled) so headless runs can confirm it fired.
- Keep it cheap for mobile (no per-number widget creation; pooled).

## Task B — `Ember.Autopilot` continuous end-to-end driver (dev builds)
Implement an autopilot that runs the FULL loop deterministically in ONE continuous session that actually travels between maps. It must survive `OpenLevel` travel (store autopilot phase in a `UGameInstanceSubsystem` or the GameInstance so each map's game mode resumes the correct step on `BeginPlay`).

`Ember.Autopilot 1` (start), driving this sequence with a small state machine + timers, logging each step:
1. **Camp (boot):** `EMBER: AUTOPILOT phase=CampStart` → grant a little starter resource if needed → after 2 s issue travel to Expedition (`SceneFlowSubsystem::TravelToExpedition`). Log `AUTOPILOT travel=Expedition`.
2. **Expedition:** `AUTOPILOT phase=Expedition` → auto-pick upgrades on (reuse `Ember.AutoPickUpgrades`) → auto-advance rooms 1→6 (you may force room clears/advances via a debug path so no player navigation is needed; whatever's most robust) → at room 7 spawn boss and auto-damage it to death (reuse `Ember.DebugAutoDamageBoss`) → on `RunResult Victory` auto-return to Camp (`TravelToCamp`). Log `AUTOPILOT roomsCleared`, `AUTOPILOT bossKilled`, `AUTOPILOT travel=Camp`.
3. **Camp (return):** `AUTOPILOT phase=CampReturn` → confirm `RunBanked` happened → buy campfire-core and grim-forge (reuse `Ember.BuyBuilding`) → grant + assign a defender monster → run Defense and auto-complete it (`Ember.DebugAutoDefense 1`) → `SimulateOffline 120` → `Save`. Log each: `AUTOPILOT bought=…`, `AUTOPILOT defenseDone`, `AUTOPILOT offlineDone`, `AUTOPILOT saved`.
4. **Finish:** `EMBER: AUTOPILOT COMPLETE` (single terminal line). Optionally auto-quit after a short delay in unattended mode.

The autopilot MUST work when launched as: `UnrealEditor.exe <proj> /Game/Emberbound/Maps/Camp/L_Camp?game=/Script/EmberboundIsekai.CampGameMode -game -RenderOffScreen -nosplash -stdout -resx=1280 -resy=720 -ExecCmds="Ember.Autopilot 1"` (no `-unattended`; kill only your own PID after `AUTOPILOT COMPLETE`).

## Task C — Fix whatever the continuous run exposes
Run the autopilot end-to-end. If banking, travel handoff, save state, or resume-after-travel breaks (things per-stage tests couldn't catch), fix the root cause and re-run until the full chain logs `AUTOPILOT COMPLETE` with all intermediate steps and no `Fatal`/`Error: [C++]`.

## Verify + report
1. UBT green (both targets). 2. Autopilot headless run: paste the ordered `EMBER: AUTOPILOT …` marker sequence (CampStart→travel=Expedition→Expedition→roomsCleared→bossKilled→travel=Camp→CampReturn→bought→defenseDone→offlineDone→saved→COMPLETE) + a `DamageText` marker. 3. Reboot after autopilot: `SaveLoaded` reflects the upgrades/resources the autopilot produced. 4. `Docs/orchestration/M10_report.md` + TEST_REPORT rows. 5. Commit `M10: damage numbers + continuous end-to-end autopilot`. Print `M10_DONE`.
