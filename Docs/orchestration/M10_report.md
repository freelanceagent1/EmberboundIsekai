# M10 Report - Continuous autopilot + damage numbers

## Implemented
- Added pooled floating damage text using `UEmberDamageTextSubsystem`, `AEmberDamageTextActor`, and the existing `UObjectPoolSubsystem`.
- Hooked `UHealthComponent::ApplyDamage` so player, enemy, boss, campfire, defender, companion, projectile, and status damage share the same pooled feedback path.
- Added `UEmberAutopilotSubsystem` with `Ember.Autopilot 1`, persistent phase state across real `OpenLevel` travel, and direct system calls for travel, run clear, boss kill, banking, building buys, monster assignment, defense, offline simulation, and save.
- Hardened delayed title/monster toast removal timers with weak widget pointers after the continuous run exposed a stale-widget crash across map travel.
- Added M10 automation coverage for damage-text policy and the expected autopilot marker chain.

## Verification
- UBT editor target: `EmberboundIsekaiEditor Win64 Development`, `-MaxParallelActions=10` -> `Result: Succeeded`.
- UBT game target: `EmberboundIsekai Win64 Development`, `-MaxParallelActions=10` -> `Result: Succeeded`.
- Continuous autopilot command:
  - `UnrealEditor.exe EmberboundIsekai.uproject /Game/Emberbound/Maps/Camp/L_Camp?game=/Script/EmberboundIsekai.CampGameMode -game -RenderOffScreen -nosplash -stdout -resx=1280 -resy=720 -ExecCmds="Ember.Autopilot 1"`
  - Final log: `Saved/Logs/EmberboundIsekai-backup-2026.07.08-19.46.34.log`
  - No `Fatal` or `Error: [C++]` marker in the final successful log.

## Ordered autopilot marker chain
```text
EMBER: AUTOPILOT phase=CampStart
EMBER: AUTOPILOT travel=Expedition
EMBER: AUTOPILOT phase=Expedition
EMBER: AUTOPILOT roomsCleared
EMBER: AUTOPILOT bossKilled
EMBER: AUTOPILOT travel=Camp
EMBER: RunBanked Gold=65 Wood=20 SoulAsh=15 Embers=0 Shards=0
EMBER: AUTOPILOT phase=CampReturn
EMBER: AUTOPILOT bought=campfire-core
EMBER: AUTOPILOT bought=grim-forge
EMBER: MonsterAssigned bone-rat role=defender slots=1/1 defender=4.0
EMBER: DefenderSpawned bone-rat
EMBER: AUTOPILOT defenseDone
EMBER: AUTOPILOT offlineDone
EMBER: AUTOPILOT saved
EMBER: AUTOPILOT COMPLETE
```

Damage text evidence from the same run:
```text
EMBER: DamageText n=1
EMBER: DamageText n=5
```

Fresh boot after autopilot:
```text
EMBER: SaveLoaded Gold=512 Wood=220 SoulAsh=122 campfire=1 forge=1 slimeShards=203 owned=2 assignments=1 titles=2 volume=1.00 joystick=1
```

## Deviations / notes
- The first continuous run exposed a real crash in delayed title toast cleanup after level travel. Root cause was a raw widget pointer captured by a timer lambda; fixed with `TWeakObjectPtr` for title and monster unlock toasts.
- The first defender assignment attempt used `slime`, but M6 data marks slime as worker/companion only. Autopilot now grants and assigns `bone-rat`, which is defender-capable.
- The monitor script initially watched stdout only; `LogEmberbound` markers were complete in the project log. Final verification uses the project log.
