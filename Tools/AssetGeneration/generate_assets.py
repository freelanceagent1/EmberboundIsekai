"""
generate_assets.py - Orchestrates the Emberbound Isekai ComfyUI asset batch.

Pipeline:
  1. Probe the ComfyUI server and confirm the SDXL checkpoint is visible.
  2. Iterate prompts.json (optionally filtered by --category/--ids).
  3. For each asset: submit the txt2img workflow, wait for completion,
     save the raw PNG to GeneratedAssets/staging/<category>/<id>.png.
     On failure, retry once with `simplified_prompt`. On second failure,
     fall back to procedural_fallback.py and mark the asset accordingly.
  4. Post-process (Pillow) every staged image (real or fallback) into
     GeneratedAssets/processed/<category>/ per category rules:
       - alpha_key categories: magenta flood-fill-from-corners keying,
         crop-to-content, center on a square canvas, resize to each
         target size.
       - environment: simple mirror-blend seam softening, resize.
       - keyart: resize/stretch to the final marketing dimensions.
  5. Write/update asset_manifest.json and print a summary table.
  6. Regenerate the root ASSET_MANIFEST.md table from the manifest.

This script ONLY talks to the ComfyUI HTTP/WS API and the local
filesystem. It never imports `unreal` and never launches any Unreal
process. Safe to re-run; existing staged PNGs are reused unless
--force is passed (so a partial/interrupted run can resume cheaply,
and README-documented "re-run one category" workflows are fast).
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import time
import traceback
from datetime import datetime, timezone

from PIL import Image, ImageChops, ImageDraw, ImageFilter
import numpy as np

try:
    from scipy import ndimage as _ndimage
    HAVE_SCIPY = True
except ImportError:
    HAVE_SCIPY = False

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import comfy_client  # noqa: E402
import procedural_fallback  # noqa: E402

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ASSET_GEN_DIR = os.path.dirname(os.path.abspath(__file__))
PROMPTS_PATH = os.path.join(ASSET_GEN_DIR, "prompts.json")
WORKFLOW_TEMPLATE_PATH = os.path.join(REPO_ROOT, "Tools", "ComfyWorkflows", "sdxl_txt2img_api.json")
STAGING_ROOT = os.path.join(REPO_ROOT, "GeneratedAssets", "staging")
PROCESSED_ROOT = os.path.join(REPO_ROOT, "GeneratedAssets", "processed")
MANIFEST_JSON_PATH = os.path.join(ASSET_GEN_DIR, "asset_manifest.json")
MANIFEST_MD_PATH = os.path.join(REPO_ROOT, "ASSET_MANIFEST.md")

MAGENTA = np.array([255, 0, 255])
MAGENTA_HUE_PIL = 212  # pure magenta hue in PIL's 0-255 HSV hue scale (== 300 deg / 360 * 255)
MAGENTA_HUE_TOLERANCE_PIL = 34  # +/- ~48 degrees; SDXL renders "magenta" background with a
# shadow/highlight gradient that skews it toward pink/plum (observed hue ~325-340 in testing,
# i.e. drifted ~15-40deg from pure 300deg magenta) rather than a flat (255,0,255), so keying
# on raw RGB distance to pure magenta misses most of the background. Hue is far more stable
# across that lighting gradient than RGB distance, so we key on hue + saturation/value floors.
MAGENTA_SAT_MIN = 40   # 0-255 PIL saturation floor (reject near-gray pixels)
MAGENTA_VAL_MIN = 10   # 0-255 PIL value floor (reject near-black pixels, hue undefined there)
GEN_TIMEOUT_SEC = 90.0

CATEGORY_PATH_MAP = {
    "characters": "Characters",
    "buildings": "Buildings",
    "environment": "Environment",
    "ui-icons": "UIIcons",
    "upgrade-icons": "UpgradeIcons",
    "vfx": "VFX",
    "keyart": "KeyArt",
}

CATEGORY_USAGE_NOTE = {
    "characters": "Character billboard sprite / portrait source texture",
    "buildings": "Camp building sprite/icon source texture",
    "environment": "Tileable-ish ground/wall/decal texture",
    "ui-icons": "HUD resource/stat icon",
    "upgrade-icons": "Upgrade card icon",
    "vfx": "VFX sprite (projectile/impact/pickup) source texture",
    "keyart": "Title screen / marketing key art",
}


def log(msg: str) -> None:
    ts = datetime.now().strftime("%H:%M:%S")
    print(f"[{ts}] {msg}", flush=True)


def to_pascal(id_kebab: str) -> str:
    return "".join(part.capitalize() for part in id_kebab.split("-"))


def ue_path_for(asset_id: str, category: str) -> str:
    return f"/Game/Emberbound/Generated/{CATEGORY_PATH_MAP[category]}/T_{to_pascal(asset_id)}"


def relpath(p: str) -> str:
    return os.path.relpath(p, REPO_ROOT).replace("\\", "/")


# ----------------------------------------------------------------------
# Post-processing
# ----------------------------------------------------------------------

def _border_connected_mask(candidate: np.ndarray) -> np.ndarray:
    """
    Given a boolean 2D candidate mask, return the subset that is
    8-connected to the image border (i.e. actually reaches an edge, not
    an isolated island of matching pixels fully enclosed by the subject).
    Shared by every keying strategy below so "only remove background that
    touches the frame edge" stays a single implementation.
    """
    if HAVE_SCIPY:
        structure = np.ones((3, 3), dtype=int)
        labeled, _num = _ndimage.label(candidate, structure=structure)
        border_labels = set(labeled[0, :]) | set(labeled[-1, :]) | set(labeled[:, 0]) | set(labeled[:, -1])
        border_labels.discard(0)
        return np.isin(labeled, list(border_labels)) if border_labels else np.zeros_like(candidate)

    # Fallback: simple BFS flood fill from the four corners.
    h, w = candidate.shape
    visited = np.zeros_like(candidate)
    stack = [(0, 0), (0, w - 1), (h - 1, 0), (h - 1, w - 1)]
    seen = set(stack)
    while stack:
        y, x = stack.pop()
        if not candidate[y, x] or visited[y, x]:
            continue
        visited[y, x] = True
        for ny, nx in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1)):
            if 0 <= ny < h and 0 <= nx < w and (ny, nx) not in seen and candidate[ny, nx] and not visited[ny, nx]:
                seen.add((ny, nx))
                stack.append((ny, nx))
    return visited


def _magenta_candidate_mask(rgb: Image.Image) -> np.ndarray:
    """Boolean mask of pixels whose HUE is near-magenta (see module docstring
    on alpha_key_magenta for why hue, not RGB distance)."""
    hsv = rgb.convert("HSV")
    h_arr, s_arr, v_arr = (np.asarray(band).astype(np.int16) for band in hsv.split())
    hue_diff = np.abs(h_arr - MAGENTA_HUE_PIL)
    hue_diff = np.minimum(hue_diff, 255 - hue_diff)  # circular distance
    return (hue_diff <= MAGENTA_HUE_TOLERANCE_PIL) & (s_arr >= MAGENTA_SAT_MIN) & (v_arr >= MAGENTA_VAL_MIN)


FLAT_BG_COLOR_TOLERANCE = 34   # euclidean RGB distance tolerance for corner flood-fill keying
FLAT_BG_CORNER_STD_MAX = 42    # corners must be this uniform to be trusted as a flat backdrop
FLAT_BG_CORNER_FRAC = 0.05     # corner sample size (fraction of min(width,height)) for the reference color


def _flat_background_candidate_mask(rgb: Image.Image) -> np.ndarray:
    """
    Generic near-uniform-background flood fill. Complements the hue-based
    magenta keyer for renders where SDXL ignored the "magenta background"
    instruction outright and came back on a flat WHITE/GREY (or any other
    near-solid) backdrop instead -- see Docs/orchestration/M5a_QA_notes.md
    (ui-icons/gold_512: white background + coin scatter, silently passed
    through fully opaque by the magenta-only keyer because it isn't
    magenta hue at all).

    Samples a small block in each of the 4 corners, takes their pooled
    median as a background reference color, and -- only if the corners are
    actually near-uniform (this really looks like a flat backdrop, not
    busy subject detail poking into a corner, and not a strong magenta
    lighting gradient which pools to a high corner-to-corner std) --
    flags every pixel within FLAT_BG_COLOR_TOLERANCE of that reference.
    Border-connection filtering happens in the caller, same as the
    magenta mask, so this can never eat into subject detail that doesn't
    touch the frame edge.
    """
    arr = np.asarray(rgb).astype(np.int16)
    h, w, _ = arr.shape
    cs = max(3, int(min(h, w) * FLAT_BG_CORNER_FRAC))
    corner_blocks = [
        arr[0:cs, 0:cs], arr[0:cs, w - cs:w],
        arr[h - cs:h, 0:cs], arr[h - cs:h, w - cs:w],
    ]
    corner_pixels = np.concatenate([c.reshape(-1, 3) for c in corner_blocks], axis=0)
    if corner_pixels.std(axis=0).mean() > FLAT_BG_CORNER_STD_MAX:
        # Corners disagree too much to trust a single reference color
        # (typical of the magenta lighting gradient, or a subject that
        # fills a corner) -- bail out, let the magenta mask handle it.
        return np.zeros((h, w), dtype=bool)

    ref_color = np.median(corner_pixels, axis=0)
    dist = np.sqrt(((arr - ref_color) ** 2).sum(axis=2))
    return dist <= FLAT_BG_COLOR_TOLERANCE


def alpha_key_magenta(img: Image.Image) -> Image.Image:
    """
    Flood-fill-from-corners magenta background removal.

    Keys on HUE (not raw RGB distance to pure (255,0,255)) because
    SDXL renders the "plain flat magenta background" instruction with
    a soft shadow/highlight gradient rather than a flat color -- the
    hue stays close to magenta across that gradient even though
    saturation/value swing widely, so hue+floors is far more reliable
    than a euclidean RGB threshold.

    Any near-magenta-hue pixel that is connected (8-connectivity) to
    the image border becomes transparent; near-magenta-hue pixels
    fully enclosed by the subject (e.g. a purple/soul-ash detail in
    the artwork) are left opaque since they don't touch the border.

    Kept standalone (in addition to alpha_key_smart below) for direct
    reuse/testing; production post-processing calls alpha_key_smart.
    """
    rgb = img.convert("RGB")
    candidate = _magenta_candidate_mask(rgb)
    bg_mask = _border_connected_mask(candidate)

    alpha = np.where(bg_mask, 0, 255).astype(np.uint8)
    alpha_img = Image.fromarray(alpha, mode="L").filter(ImageFilter.GaussianBlur(radius=1.0))
    rgba = rgb.convert("RGBA")
    rgba.putalpha(alpha_img)
    return rgba


def alpha_key_smart(img: Image.Image) -> Image.Image:
    """
    Combined background keyer used by post_process for all alpha_key
    categories: magenta-hue keying OR'd with a generic near-uniform
    corner-color flood fill (alpha_key_magenta + _flat_background_candidate_mask),
    so a render that ignored the "magenta background" instruction and came
    back on a flat white/grey/other solid backdrop still gets keyed instead
    of silently passing through fully opaque (M5a QA fix pass,
    Docs/orchestration/M5a_QA_notes.md). Both candidate masks are restricted
    to border-connected regions before being OR'd together, so this is
    strictly additive versus alpha_key_magenta alone -- it can only remove
    *more* border-touching background, never eat into interior subject
    detail that doesn't reach the frame edge.
    """
    rgb = img.convert("RGB")
    magenta_mask = _border_connected_mask(_magenta_candidate_mask(rgb))
    flat_mask = _border_connected_mask(_flat_background_candidate_mask(rgb))
    bg_mask = magenta_mask | flat_mask

    alpha = np.where(bg_mask, 0, 255).astype(np.uint8)
    alpha_img = Image.fromarray(alpha, mode="L").filter(ImageFilter.GaussianBlur(radius=1.0))
    rgba = rgb.convert("RGBA")
    rgba.putalpha(alpha_img)
    return rgba


def crop_to_content(img: Image.Image, padding_frac: float = 0.04) -> Image.Image:
    """Crop an RGBA image to its non-transparent bounding box, with padding, then
    pad to a square canvas centered on that content."""
    alpha = np.asarray(img.split()[-1])
    ys, xs = np.where(alpha > 8)
    if len(xs) == 0 or len(ys) == 0:
        # Fully transparent / degenerate generation; return as-is.
        return img

    x0, x1 = xs.min(), xs.max()
    y0, y1 = ys.min(), ys.max()
    w, h = img.size
    pad_x = int((x1 - x0) * padding_frac) + 2
    pad_y = int((y1 - y0) * padding_frac) + 2
    x0 = max(0, x0 - pad_x)
    y0 = max(0, y0 - pad_y)
    x1 = min(w - 1, x1 + pad_x)
    y1 = min(h - 1, y1 + pad_y)

    cropped = img.crop((x0, y0, x1 + 1, y1 + 1))
    side = max(cropped.size)
    canvas = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    off_x = (side - cropped.width) // 2
    off_y = (side - cropped.height) // 2
    canvas.paste(cropped, (off_x, off_y), cropped)
    return canvas


def make_tileable_ish(img: Image.Image, band_frac: float = 0.06) -> Image.Image:
    """
    Cheap seam-softening for 'tileable-ish' ground/wall textures: wrap
    the image by half its size (moving the original hard edges to the
    center), then blend a blurred copy along the new center seam so the
    wrap discontinuity is softened. Not true frequency-domain tiling,
    but enough for a stylized mobile background layer.
    """
    w, h = img.size
    rgb = img.convert("RGB")
    offset = ImageChops.offset(rgb, w // 2, h // 2)
    blur_radius = max(4, w // 64)
    blurred = offset.filter(ImageFilter.GaussianBlur(radius=blur_radius))

    mask = Image.new("L", (w, h), 0)
    mdraw = ImageDraw.Draw(mask)
    band_w = max(2, int(w * band_frac))
    band_h = max(2, int(h * band_frac))
    mdraw.rectangle([w // 2 - band_w, 0, w // 2 + band_w, h], fill=255)
    mdraw.rectangle([0, h // 2 - band_h, w, h // 2 + band_h], fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(radius=max(2, band_w // 2)))

    return Image.composite(blurred, offset, mask)


def save_variants(img: Image.Image, out_dir: str, asset_id: str, target_sizes: list) -> list:
    """Resize `img` to each entry in target_sizes and save. Returns list of abs paths."""
    os.makedirs(out_dir, exist_ok=True)
    saved = []
    for size in target_sizes:
        if isinstance(size, (list, tuple)):
            tw, th = size
            suffix = f"{tw}x{th}"
        else:
            tw = th = size
            suffix = str(size)
        resized = img.resize((tw, th), Image.LANCZOS)
        out_path = os.path.join(out_dir, f"{asset_id}_{suffix}.png")
        resized.save(out_path, "PNG")
        saved.append(out_path)
    return saved


def post_process(raw_path: str, asset: dict) -> list:
    """Run category-appropriate post-processing. Returns list of absolute processed paths."""
    category = asset["category"]
    asset_id = asset["id"]
    target_sizes = asset["target_sizes"]
    out_dir = os.path.join(PROCESSED_ROOT, category)

    img = Image.open(raw_path)

    if asset.get("alpha_key"):
        keyed = alpha_key_smart(img)
        content = crop_to_content(keyed)
        return save_variants(content, out_dir, asset_id, target_sizes)

    if category == "environment":
        tileable = make_tileable_ish(img)
        return save_variants(tileable, out_dir, asset_id, target_sizes)

    # keyart / other non-alpha: straight resize/stretch to final size(s).
    rgb = img.convert("RGB")
    return save_variants(rgb, out_dir, asset_id, target_sizes)


# ----------------------------------------------------------------------
# Generation
# ----------------------------------------------------------------------

def generate_one(client: comfy_client.ComfyClient, template: dict, ckpt_name: str, asset: dict,
                  staging_dir: str, force: bool) -> dict:
    """
    Returns a manifest-entry dict for this asset. Never raises; all
    failures are caught and converted into a 'fallback' or 'failed'
    manifest entry so the batch keeps going.
    """
    asset_id = asset["id"]
    category = asset["category"]
    raw_path = os.path.join(staging_dir, f"{asset_id}.png")
    t_start = time.time()

    entry = {
        "id": asset_id,
        "category": category,
        "subject": asset.get("subject", ""),
        "prompt": asset["prompt"],
        "negative": asset["negative"],
        "seed": asset["seed"],
        "width": asset["width"],
        "height": asset["height"],
        "target_sizes": asset["target_sizes"],
        "ue_path": ue_path_for(asset_id, category),
        "usage_note": CATEGORY_USAGE_NOTE.get(category, ""),
        "generated_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
    }

    if not force and os.path.exists(raw_path):
        log(f"  [{asset_id}] staging PNG already exists, reusing (use --force to regenerate)")
        entry["source_file"] = relpath(raw_path)
        entry["status"] = "generated" if os.path.getsize(raw_path) > 0 else "fallback"
        entry["attempts"] = 0
        entry["elapsed_sec"] = 0.0
        try:
            processed = post_process(raw_path, asset)
            entry["processed_files"] = [relpath(p) for p in processed]
        except Exception as exc:  # noqa: BLE001
            log(f"  [{asset_id}] post-process FAILED on cached staging file: {exc}")
            entry["status"] = "failed"
            entry["error"] = str(exc)
        return entry

    os.makedirs(staging_dir, exist_ok=True)

    attempts = 0
    result = None
    prompts_to_try = [asset["prompt"], asset["simplified_prompt"]]

    for attempt_idx, prompt_text in enumerate(prompts_to_try, start=1):
        attempts += 1
        log(f"  [{asset_id}] attempt {attempt_idx}/2 (category={category}, seed={asset['seed']})")
        try:
            workflow = comfy_client.build_workflow(
                template=template,
                ckpt_name=ckpt_name,
                positive=prompt_text,
                negative=asset["negative"],
                width=asset["width"],
                height=asset["height"],
                seed=asset["seed"],
                filename_prefix=f"ember_{category}_{asset_id}",
            )
            result = client.generate_image(workflow, timeout=GEN_TIMEOUT_SEC)
        except Exception as exc:  # noqa: BLE001
            log(f"  [{asset_id}] attempt {attempt_idx} raised: {exc}")
            result = comfy_client.GenerationResult(ok=False, error=str(exc))

        if result.ok:
            break
        log(f"  [{asset_id}] attempt {attempt_idx} failed: {result.error}")

    if result and result.ok:
        try:
            img_info = result.images[0]
            img_bytes = client.fetch_image_bytes(
                img_info["filename"], img_info.get("subfolder", ""), img_info.get("type", "output")
            )
            with open(raw_path, "wb") as f:
                f.write(img_bytes)
            entry["status"] = "generated"
            entry["attempts"] = attempts
            entry["elapsed_sec"] = round(time.time() - t_start, 2)
            entry["source_file"] = relpath(raw_path)
            entry["comfy_prompt_id"] = result.prompt_id
        except Exception as exc:  # noqa: BLE001
            log(f"  [{asset_id}] failed to fetch/save generated image: {exc}")
            result = comfy_client.GenerationResult(ok=False, error=f"fetch/save failed: {exc}")

    if not (result and result.ok):
        log(f"  [{asset_id}] all ComfyUI attempts failed -> procedural fallback")
        try:
            placeholder = procedural_fallback.generate_placeholder(
                asset_id, category, asset["width"], asset["height"], asset.get("alpha_key", False)
            )
            placeholder.save(raw_path, "PNG")
            entry["status"] = "fallback"
            entry["attempts"] = attempts
            entry["elapsed_sec"] = round(time.time() - t_start, 2)
            entry["source_file"] = relpath(raw_path)
            entry["error"] = result.error if result else "unknown generation failure"
        except Exception as exc:  # noqa: BLE001
            log(f"  [{asset_id}] PROCEDURAL FALLBACK ALSO FAILED: {exc}")
            entry["status"] = "failed"
            entry["attempts"] = attempts
            entry["elapsed_sec"] = round(time.time() - t_start, 2)
            entry["error"] = str(exc)
            return entry

    try:
        processed = post_process(raw_path, asset)
        entry["processed_files"] = [relpath(p) for p in processed]
    except Exception as exc:  # noqa: BLE001
        log(f"  [{asset_id}] post-process FAILED: {exc}\n{traceback.format_exc()}")
        entry["status"] = "failed"
        entry["error"] = f"post-process failed: {exc}"

    return entry


# ----------------------------------------------------------------------
# Manifest
# ----------------------------------------------------------------------

def load_existing_manifest() -> dict:
    if os.path.exists(MANIFEST_JSON_PATH):
        with open(MANIFEST_JSON_PATH, "r", encoding="utf-8") as f:
            return json.load(f)
    return {"generated_at": None, "checkpoint": None, "assets": {}}


def write_manifest_json(manifest: dict) -> None:
    with open(MANIFEST_JSON_PATH, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)


def write_manifest_md(manifest: dict) -> None:
    assets = manifest["assets"]
    lines = [
        "# ASSET_MANIFEST.md",
        "",
        "Every generated asset: prompt, source file, target UE path, usage. "
        "Populated at Milestone 5 by `Tools/AssetGeneration/generate_assets.py` "
        "(machine-readable twin: `Tools/AssetGeneration/asset_manifest.json`).",
        "",
        "| Asset | Category | Status | Prompt (short) | Source PNG | UE path | Used by |",
        "|---|---|---|---|---|---|---|",
    ]
    for asset_id in sorted(assets.keys(), key=lambda k: (assets[k]["category"], k)):
        a = assets[asset_id]
        short_prompt = a.get("subject") or a["prompt"].split(",")[0]
        if len(short_prompt) > 70:
            short_prompt = short_prompt[:67] + "..."
        source = a.get("source_file", "")
        lines.append(
            f"| {asset_id} | {a['category']} | {a['status']} | {short_prompt} | {source} | "
            f"{a['ue_path']} | {a.get('usage_note', '')} |"
        )

    gen = sum(1 for a in assets.values() if a["status"] == "generated")
    fb = sum(1 for a in assets.values() if a["status"] == "fallback")
    fail = sum(1 for a in assets.values() if a["status"] == "failed")

    lines += [
        "",
        "## Generation status log",
        f"- {manifest.get('generated_at', '')}: batch run via `generate_assets.py` against checkpoint "
        f"`{manifest.get('checkpoint', '')}`. {gen} generated, {fb} procedural fallback, {fail} failed "
        f"out of {len(assets)} total assets. Full run log / timings: `Docs/orchestration/M5a_report.md`.",
    ]

    with open(MANIFEST_MD_PATH, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Generate Emberbound Isekai art assets via ComfyUI.")
    parser.add_argument("--category", help="Comma-separated category filter (e.g. ui-icons,vfx)")
    parser.add_argument("--ids", help="Comma-separated asset id filter (e.g. gold,wood)")
    parser.add_argument("--force", action="store_true", help="Regenerate even if a staged PNG already exists")
    parser.add_argument("--server", default="http://127.0.0.1:8188", help="ComfyUI base URL")
    parser.add_argument("--list", action="store_true", help="List categories/ids and exit")
    args = parser.parse_args()

    with open(PROMPTS_PATH, "r", encoding="utf-8") as f:
        prompts_doc = json.load(f)

    all_assets = prompts_doc["assets"]
    checkpoint_name = prompts_doc["checkpoint"]

    if args.list:
        for a in all_assets:
            print(f"{a['category']:>16} | {a['id']}")
        return

    category_filter = set(args.category.split(",")) if args.category else None
    id_filter = set(args.ids.split(",")) if args.ids else None

    selected = [
        a for a in all_assets
        if (category_filter is None or a["category"] in category_filter)
        and (id_filter is None or a["id"] in id_filter)
    ]
    if not selected:
        print("No assets matched the given --category/--ids filters.", file=sys.stderr)
        sys.exit(1)

    log(f"Selected {len(selected)}/{len(all_assets)} assets for this run.")

    client = comfy_client.ComfyClient(base_url=args.server)

    log("Probing ComfyUI server...")
    stats = client.probe_server()
    log(f"  server OK: ComfyUI {stats.get('system', {}).get('comfyui_version')}, "
        f"device={stats.get('devices', [{}])[0].get('name', '?')}")

    log(f"Confirming checkpoint '{checkpoint_name}' via /object_info...")
    client.ensure_checkpoint(checkpoint_name)
    log("  checkpoint OK")

    template = comfy_client.load_workflow_template(WORKFLOW_TEMPLATE_PATH)

    manifest = load_existing_manifest()
    manifest["checkpoint"] = checkpoint_name

    batch_start = time.time()
    results = []
    for i, asset in enumerate(selected, start=1):
        log(f"[{i}/{len(selected)}] {asset['category']}/{asset['id']}")
        staging_dir = os.path.join(STAGING_ROOT, asset["category"])
        entry = generate_one(client, template, checkpoint_name, asset, staging_dir, args.force)
        results.append(entry)
        manifest["assets"][asset["id"]] = entry

    batch_elapsed = time.time() - batch_start
    manifest["generated_at"] = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    manifest["last_batch_elapsed_sec"] = round(batch_elapsed, 2)
    manifest["last_batch_asset_count"] = len(selected)

    write_manifest_json(manifest)
    write_manifest_md(manifest)

    # Summary table
    gen = [r for r in results if r["status"] == "generated"]
    fb = [r for r in results if r["status"] == "fallback"]
    fail = [r for r in results if r["status"] == "failed"]

    print("\n" + "=" * 72)
    print(f"BATCH SUMMARY  ({len(results)} assets, {batch_elapsed:.1f}s wall time)")
    print("=" * 72)
    print(f"{'status':<12}{'count':<8}{'ids'}")
    print(f"{'generated':<12}{len(gen):<8}{', '.join(r['id'] for r in gen[:12])}{' ...' if len(gen) > 12 else ''}")
    print(f"{'fallback':<12}{len(fb):<8}{', '.join(r['id'] for r in fb)}")
    print(f"{'failed':<12}{len(fail):<8}{', '.join(r['id'] for r in fail)}")
    print("=" * 72)
    print(f"Manifest JSON: {MANIFEST_JSON_PATH}")
    print(f"Manifest MD:   {MANIFEST_MD_PATH}")


if __name__ == "__main__":
    main()
