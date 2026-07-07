"""
qa_audit.py - Programmatic alpha-channel QA audit for processed sprite/icon
assets (Emberbound Isekai asset pipeline, M5a QA fix pass).

For every PNG under GeneratedAssets/processed/<category>/ (category
restricted to --categories, default: ui-icons, upgrade-icons, characters,
vfx), measures:

  - Per-corner (top-left/top-right/bottom-left/bottom-right, each a box of
    `--corner-frac` * width by `--corner-frac` * height, default 12%) mean
    alpha and "opaque ratio" (fraction of pixels with alpha above
    --opaque-threshold, default 200/255).
  - Overall image opaque-pixel ratio.

Verdict: FAIL if the worst (max) corner's opaque ratio exceeds
--fail-ratio (default 0.20) -- i.e. background survived keying in at least
one corner. PASS otherwise.

This does not modify any file. It only reads processed/<category>/*.png
and prints/optionally writes a JSON report, so it's safe to run before and
after a re-keying pass to diff results.

Usage (from Tools/AssetGeneration/):
    C:\\Python314\\python.exe qa_audit.py
    C:\\Python314\\python.exe qa_audit.py --categories ui-icons,characters --out before.json
"""

from __future__ import annotations

import argparse
import json
import os
import sys

from PIL import Image
import numpy as np

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PROCESSED_ROOT = os.path.join(REPO_ROOT, "GeneratedAssets", "processed")

DEFAULT_CATEGORIES = ["ui-icons", "upgrade-icons", "characters", "vfx"]
CORNER_FRAC = 0.12
OPAQUE_THRESHOLD = 200
FAIL_RATIO = 0.20  # a corner counts as "background survived" above this opaque-pixel fraction


def corner_boxes(w: int, h: int, frac: float) -> dict:
    cw = max(1, int(round(w * frac)))
    ch = max(1, int(round(h * frac)))
    return {
        "TL": (0, 0, cw, ch),
        "TR": (w - cw, 0, w, ch),
        "BL": (0, h - ch, cw, h),
        "BR": (w - cw, h - ch, w, h),
    }


def audit_image(path: str, corner_frac: float, opaque_threshold: int, fail_ratio: float) -> dict:
    img = Image.open(path).convert("RGBA")
    w, h = img.size
    alpha = np.asarray(img.split()[-1])

    corners = {}
    for name, (x0, y0, x1, y1) in corner_boxes(w, h, corner_frac).items():
        region = alpha[y0:y1, x0:x1]
        corners[name] = {
            "mean_alpha": round(float(region.mean()), 1),
            "opaque_ratio": round(float((region > opaque_threshold).mean()), 4),
        }

    overall_opaque_ratio = round(float((alpha > opaque_threshold).mean()), 4)
    max_corner_opaque = max(c["opaque_ratio"] for c in corners.values())
    worst_corner = max(corners, key=lambda k: corners[k]["opaque_ratio"])
    verdict = "FAIL" if max_corner_opaque > fail_ratio else "PASS"

    return {
        "path": os.path.relpath(path, REPO_ROOT).replace("\\", "/"),
        "size": [w, h],
        "corners": corners,
        "overall_opaque_ratio": overall_opaque_ratio,
        "max_corner_opaque_ratio": max_corner_opaque,
        "worst_corner": worst_corner,
        "verdict": verdict,
    }


def main():
    parser = argparse.ArgumentParser(description="Alpha-channel corner-opacity QA audit.")
    parser.add_argument("--categories", default=",".join(DEFAULT_CATEGORIES),
                         help="Comma-separated category list (dirs under GeneratedAssets/processed/)")
    parser.add_argument("--corner-frac", type=float, default=CORNER_FRAC)
    parser.add_argument("--opaque-threshold", type=int, default=OPAQUE_THRESHOLD)
    parser.add_argument("--fail-ratio", type=float, default=FAIL_RATIO)
    parser.add_argument("--out", default=None, help="Optional path to write a JSON report")
    parser.add_argument("--ids", default=None, help="Comma-separated asset-id substrings to restrict to (matches processed filename)")
    args = parser.parse_args()

    categories = [c.strip() for c in args.categories.split(",") if c.strip()]
    id_filter = [s.strip() for s in args.ids.split(",")] if args.ids else None

    results = []
    for category in categories:
        cat_dir = os.path.join(PROCESSED_ROOT, category)
        if not os.path.isdir(cat_dir):
            print(f"WARNING: no such processed category dir: {cat_dir}", file=sys.stderr)
            continue
        for fname in sorted(os.listdir(cat_dir)):
            if not fname.lower().endswith(".png"):
                continue
            if id_filter and not any(s in fname for s in id_filter):
                continue
            path = os.path.join(cat_dir, fname)
            entry = audit_image(path, args.corner_frac, args.opaque_threshold, args.fail_ratio)
            entry["category"] = category
            entry["filename"] = fname
            results.append(entry)

    # Print table
    fails = [r for r in results if r["verdict"] == "FAIL"]
    print(f"{'category':<16}{'filename':<34}{'verdict':<8}{'max_corner':<12}{'worst':<6}{'overall_opaque'}")
    print("-" * 96)
    for r in results:
        print(f"{r['category']:<16}{r['filename']:<34}{r['verdict']:<8}"
              f"{r['max_corner_opaque_ratio']:<12}{r['worst_corner']:<6}{r['overall_opaque_ratio']}")
    print("-" * 96)
    print(f"TOTAL: {len(results)}   PASS: {len(results) - len(fails)}   FAIL: {len(fails)}")
    if fails:
        print("FAILED: " + ", ".join(r["filename"] for r in fails))

    if args.out:
        with open(args.out, "w", encoding="utf-8") as f:
            json.dump({"results": results, "fail_ratio": args.fail_ratio,
                       "opaque_threshold": args.opaque_threshold,
                       "corner_frac": args.corner_frac}, f, indent=2)
        print(f"\nJSON report written to {args.out}")


if __name__ == "__main__":
    main()
