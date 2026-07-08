# M9 Validation Plan (orchestrator-owned)

Acceptance evidence to produce before any completion claim:

1. **Compile**: both targets green from clean state — `EmberboundIsekaiEditor` and `EmberboundIsekai` (Win64 Development, `-MaxParallelActions=10`).
2. **Project opens**: editor commandlet boots have run all milestones (module loads, no rebuild prompt); additionally boot the full editor once headlessly with the asset-verify script as final gate.
3. **Playable loop in a real rendered run** (windowed `-game` on real RHI, the same runtime path as PIE):
   a. Boot → splash art → L_Camp; screenshot (HUD icons, buildings with generated art).
   b. `Ember.TravelExpedition` → combat in room 1 with auto-fire; screenshot (enemies as billboards, projectiles, pickups).
   c. Level-up → 3 upgrade cards with generated icons; screenshot; pick card.
   d. `Ember.StartRoom 7` → boss fight; screenshots (telegraph, patterns); `Ember.DebugAutoDamageBoss 1` → victory result screen screenshot.
   e. Return to camp → banked resources; buy campfire + forge upgrades via panel/debug; screenshot of upgrade panel.
   f. `Ember.GrantShards` + assign monster; `Ember.SimulateOffline 120` → offline popup screenshot.
   g. `Ember.StartDefense` → defense waves; screenshot; result.
   h. Quit → relaunch → verify `SaveLoaded` shows persisted state.
   Screenshots via Windows screenshot tooling into `Docs/validation/`; all EMBER markers collected from `Saved/Logs`.
4. **Automation tests**: run all `Emberbound.*` tests.
5. **Docs final pass**: README (controls incl. joystick, run steps), TEST_REPORT (compile table, automated checks, PIE checklist with evidence links, known issues), ASSET_MANIFEST spot-audit, BALANCE deltas, ASSUMPTIONS current.
6. **Final response** per spec section T: project path, run steps, systems, assets, tests, known issues, next tasks.

Rule: PIE items I can only verify in `-game` mode are marked as such in TEST_REPORT with a note that interactive PIE confirmation remains a 2-minute manual step for the user.
