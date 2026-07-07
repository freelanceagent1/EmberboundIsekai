"""
procedural_fallback.py - Pillow-drawn stylized placeholder generator.

Used by generate_assets.py when a ComfyUI generation fails twice
(original prompt + one simplified-prompt retry). Produces a flat-color
silhouette-style placeholder so downstream post-processing / UE import
still has a same-shaped asset to work with, and the manifest can flag
it as `status: "fallback"` for later manual/regenerated replacement.

No network calls, no Unreal dependency. Deterministic per asset id
(same id -> same placeholder every run) so re-runs are stable.

Style: flat "ember" palette silhouette shapes on a plain magenta
background for alpha-keyed categories (so it survives the same
background-removal pass as a real generation), or a plain warm-toned
gradient card for non-alpha categories (environment/keyart). No text
is drawn on the placeholder itself (per work order: "label-free");
the id is only stored in the manifest.
"""

from __future__ import annotations

import hashlib
import math
from typing import Tuple

from PIL import Image, ImageDraw, ImageFilter

MAGENTA = (255, 0, 255)

# Small palette of "ember" flat colors to cycle through deterministically.
PALETTE = [
    (214, 90, 41),    # ember orange
    (156, 60, 158),   # soul-ash purple
    (79, 143, 92),    # goblin/slime green
    (200, 178, 60),   # gold
    (94, 74, 158),    # frost/arcane violet
    (176, 46, 46),    # blood-ember red
    (60, 120, 150),   # frost blue
    (120, 90, 60),    # wood brown
]


def _color_for(asset_id: str) -> Tuple[int, int, int]:
    h = hashlib.sha1(asset_id.encode("utf-8")).hexdigest()
    idx = int(h[:8], 16) % len(PALETTE)
    return PALETTE[idx]


def _accent_for(asset_id: str) -> Tuple[int, int, int]:
    h = hashlib.sha1((asset_id + "accent").encode("utf-8")).hexdigest()
    idx = int(h[:8], 16) % len(PALETTE)
    return PALETTE[idx]


def _shape_index(asset_id: str, n: int) -> int:
    h = hashlib.sha1((asset_id + "shape").encode("utf-8")).hexdigest()
    return int(h[:8], 16) % n


def _draw_blob(draw: ImageDraw.ImageDraw, cx: int, cy: int, r: int, color, seed_int: int):
    """A soft irregular blob (approximated silhouette) built from a jittered polygon."""
    pts = []
    n = 14
    for i in range(n):
        angle = (2 * math.pi * i) / n
        jitter = 0.75 + 0.25 * math.sin(seed_int * 0.13 + i * 1.7)
        rad = r * jitter
        pts.append((cx + rad * math.cos(angle), cy + rad * math.sin(angle)))
    draw.polygon(pts, fill=color)


def generate_placeholder(asset_id: str, category: str, width: int, height: int, alpha_key: bool) -> Image.Image:
    """
    Return an RGB PIL Image (width x height) that looks like a
    intentional flat-shaded placeholder, not an error state.

    - alpha_key=True: drawn on solid magenta so the normal background
      keying step in generate_assets.py turns it into a clean sprite.
    - alpha_key=False: drawn as a soft two-tone gradient card (used for
      environment textures / keyart where no keying happens).
    """
    color = _color_for(asset_id)
    accent = _accent_for(asset_id)
    seed_int = int(hashlib.sha1(asset_id.encode("utf-8")).hexdigest()[:8], 16)

    if alpha_key:
        img = Image.new("RGB", (width, height), MAGENTA)
        draw = ImageDraw.Draw(img)
        cx, cy = width // 2, height // 2
        base_r = int(min(width, height) * 0.30)

        shape_kind = _shape_index(asset_id, 4)
        if shape_kind == 0:
            # rounded blob silhouette (creature/character-ish)
            _draw_blob(draw, cx, cy, base_r, color, seed_int)
            _draw_blob(draw, cx, cy - int(base_r * 0.15), int(base_r * 0.55), accent, seed_int + 5)
        elif shape_kind == 1:
            # diamond/icon-ish
            r = base_r
            draw.polygon([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], fill=color)
            r2 = int(r * 0.5)
            draw.polygon([(cx, cy - r2), (cx + r2, cy), (cx, cy + r2), (cx - r2, cy)], fill=accent)
        elif shape_kind == 2:
            # structure-ish: base rectangle + roof triangle (buildings)
            bw, bh = int(base_r * 1.6), int(base_r * 1.2)
            left, top = cx - bw // 2, cy - bh // 2 + int(base_r * 0.3)
            draw.rectangle([left, top, left + bw, top + bh], fill=color)
            draw.polygon(
                [(left - int(bw * 0.1), top), (cx, top - int(bh * 0.8)), (left + bw + int(bw * 0.1), top)],
                fill=accent,
            )
        else:
            # burst/vfx-ish
            r = base_r
            n_spikes = 8
            pts = []
            for i in range(n_spikes * 2):
                angle = (math.pi * i) / n_spikes
                rad = r if i % 2 == 0 else r * 0.45
                pts.append((cx + rad * math.cos(angle), cy + rad * math.sin(angle)))
            draw.polygon(pts, fill=color)
            draw.ellipse([cx - r * 0.25, cy - r * 0.25, cx + r * 0.25, cy + r * 0.25], fill=accent)

        # Thin border for readable silhouette even before/without keying.
        draw.ellipse(
            [cx - base_r * 1.35, cy - base_r * 1.35, cx + base_r * 1.35, cy + base_r * 1.35],
            outline=(30, 20, 20),
            width=0,
        )
        return img

    # Non-alpha (environment / keyart): soft diagonal gradient card, no magenta.
    img = Image.new("RGB", (width, height), color)
    top = Image.new("RGB", (width, height), accent)
    mask = Image.new("L", (width, height), 0)
    mdraw = ImageDraw.Draw(mask)
    for y in range(height):
        v = int(255 * (y / max(1, height - 1)))
        mdraw.line([(0, y), (width, y)], fill=v)
    img = Image.composite(top, img, mask)
    img = img.filter(ImageFilter.GaussianBlur(radius=max(2, width // 200)))

    draw = ImageDraw.Draw(img)
    seed_int2 = seed_int
    for i in range(6):
        cx = int(width * ((seed_int2 >> (i * 3)) % 97) / 97.0)
        cy = int(height * ((seed_int2 >> (i * 5 + 1)) % 89) / 89.0)
        r = max(6, width // 40)
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=tuple(min(255, c + 25) for c in accent))

    return img


if __name__ == "__main__":
    # Standalone smoke test.
    out = generate_placeholder("smoke-test-icon", "ui-icons", 1024, 1024, alpha_key=True)
    out.save("procedural_fallback_smoketest_alpha.png")
    out2 = generate_placeholder("smoke-test-texture", "environment", 1024, 1024, alpha_key=False)
    out2.save("procedural_fallback_smoketest_flat.png")
    print("wrote smoke test PNGs")
