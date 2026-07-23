# UI_COMPONENT_INVENTORY.md

Reusable components in `Emberbound Expanded UI.dc.html` (rendered from data via loops; each maps to a UMG widget candidate).

## Chrome
- **Review toolbar** — Review-mode switch; when on: frame aspect presets (16:9 / 20:9 cutout / 4:3 / Portrait), 125% text, high contrast, reduced motion, Reset prototype data, jump rail. Never rendered inside the simulated device frame.
- **Device frame** — fixed-aspect frame with 20:9 safe-area padding (`safeL/safeR`) and cutout indicator.
- **Top bar** — portrait, name, active title, 5-currency resource bar (tooltip per currency), Settings button.
- **Nav rail** — 9 destinations with explicit labels (Camp, Character, Skills, Arsenal, Artifacts, Pet Training, World, Bestiary, Camp Buildings). "Upgrades" never appears as an ambiguous label.

## Primitives
- **Currency chip** — icon + amount + tooltip (desc).
- **Rarity badge** — bordered label tinted by rarity (common/rare/epic/legendary).
- **PROPOSED CONTENT badge** — review-mode-only chip on proposed entities.
- **State chip** — EQUIPPED / ON / role tag / biome state (LOCKED, NEW!, AVAILABLE, CLEARED, MASTERED).
- **Progress bar** — pet XP.
- **Confirm dialog** — generic modal (title, body, resource lines, confirm/cancel; danger variant; Esc closes; focus moved into dialog on open).
- **Toast** — aria-live polite transient feedback.
- **Inline message row** — aria-live per-panel action feedback (skill/arsenal/artifact/pet/buildings).

## Cards
- **Building card** (Camp + Camp Buildings) — image, level, effect, next-level line, per-resource affordability coloring, prereq lock note.
- **Weapon card / Artifact card** — locked (silhouette), favorite star, equipped chip, selection edge.
- **Pet card** — undiscovered silhouette / discovered grayscale / tamed color, shard progress, role tag.
- **Bestiary card** — silhouette state + kind tag (COMMON/ELITE/BOSS/TAMEABLE).
- **Biome card** — tileset banner, state chip, unlock requirement.
- **Boon offer card** — rarity border, stack preview ("×1 → ×2").
- **Loot reveal card** — icon, rarity, description, action buttons, inventory-full alert.
- **Detail side panel** — sticky panel per management tab (skills node, weapon, artifact, pet, bestiary entry) with contextual actions.
- **Run HUD** — biome title, 7-pip room progress (5 rooms, elite, boss), HP/gold/ash, Retreat.

All interactive controls are semantic `<button>`s with ≥44–48px min targets, visible focus (global `:focus-visible`), and text + shape state encoding (never color alone).
