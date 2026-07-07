# WORK ORDER M3 — Boss (Tutorial Dragon, Level 3) + Run Results

Continue Emberbound Isekai (UE 5.8). M1+M2 are merged — read `Docs/orchestration/M1_report.md`, `M2_report.md` and existing code; build on it. Same hard rules (UBT green, headless python content, one editor process, pooling/perf, original content).

## Scope
### Room ladder
Rooms 1–5 normal (existing), room 6 = **elite room** (2 "Elite Goblin Stabber": GoblinStabber data ×3 HP, ×1.5 damage, ×1.3 scale, distinct tint; reward ×2), room 7 = **boss room**: Tutorial Dragon, Level 3.

### `ABossTutorialDragon` (extends enemy base or own APawn)
BALANCE.md: HP 450, contact 20. Visible boss HP bar added to HUD while alive. Pattern cycle with 1.2 s idle between:
1. **Fireball volley** — 5-projectile fan at player, 12 dmg each (pooled projectiles).
2. **Dash charge** — 1.0 s telegraph (flattened glowing rectangle/decal actor along dash line), then fast dash, 18 dmg on contact during dash.
3. **Flame ring** — 12 radial projectiles (or expanding ring hazard), 12 dmg.
Enrage <30% HP: +35% move/dash speed, volley 7 projectiles, ring 16. Log `EMBER: BossPattern <name>` each cast, `EMBER: BossEnraged`, `EMBER: BossDefeated`.

### Run results
- Victory: boss reward per BALANCE.md (25 Gold, 10 Wood, 8 Soul Ash, 30% 1 Ember, 2–3 monster shards — shards recorded in a run-result struct on GameInstance; Monster Pen consumes them in M6). Result screen (C++ widget): VICTORY, loot lines, buttons "Run Again" (reload L_Expedition) and "Return to Camp" (disabled placeholder until M4).
- Defeat (player death anywhere in run): DEFEAT screen, keep 60% of run resource tally (round down), same buttons. No hard-lock.
- Banking: during run, pickups accrue to a run-local tally (if M1 credited EconomySubsystem directly, refactor to tally); on run end bank tally (100% win / 60% defeat) + boss reward into EconomySubsystem. Temporary upgrades all reset (M2 manager handles).

### Debug hooks
CVars: `Ember.StartRoom <n>` (jump ladder), `Ember.PlayerDamageMult`, exec `Ember.KillPlayer`, `Ember.DamageBoss <n>`.

### Verification (run + report)
1. UBT green. 2. Smoke A (victory): `-game`, StartRoom 7, AutoPickUpgrades on, periodically `Ember.DamageBoss 40` via dev auto-driver; log shows all 3 pattern names + BossEnraged + BossDefeated + `EMBER: RunResult Victory` + banked amounts. 3. Smoke B (defeat): StartRoom 7, `Ember.KillPlayer` after 10 s → `EMBER: RunResult Defeat` with 60% banking. 4. `Docs/orchestration/M3_report.md` + TEST_REPORT.md row. 5. Commit `M3: boss and run results`. Print `M3_DONE`.
Out of scope: Camp map/buildings (M4), monster collection UI (M6).
