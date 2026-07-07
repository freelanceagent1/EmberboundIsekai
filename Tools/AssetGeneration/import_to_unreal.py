"""
import_to_unreal.py - UE EDITOR PYTHON SCRIPT. DO NOT RUN FROM THIS WORKER.
=============================================================================
THIS SCRIPT MUST NEVER BE EXECUTED BY THIS AGENT / WORKER SESSION.

Per work order M5(a): "another worker owns the editor" and this worker must
NEVER launch UnrealEditor, UnrealEditor-Cmd, or UBT. This file is written
(authored) as a deliverable ONLY. It imports `unreal`, which does not exist
outside a running Unreal Editor Python environment, so it will immediately
ImportError if you try to run it with a plain python.exe -- that is
intentional and NOT a bug.

It is meant to be run LATER by the orchestrator (or the worker that owns
the editor process) via:

    "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
        "E:\\mobilegamer\\EmberboundIsekai\\EmberboundIsekai.uproject" ^
        -run=pythonscript ^
        -script="E:\\mobilegamer\\EmberboundIsekai\\Tools\\AssetGeneration\\import_to_unreal.py" ^
        -stdout -unattended -nopause -nosplash

=============================================================================

What it does when run inside the editor:
  1. Reads `asset_manifest.json` (written by generate_assets.py).
  2. For every manifest entry with status "generated" or "fallback" (i.e.
     every entry that has processed PNG files -- "failed" entries are
     skipped and logged), imports the LARGEST processed PNG variant into
     the manifest's `ue_path` (e.g. /Game/Emberbound/Generated/UIIcons/T_Gold)
     using AssetToolsHelpers + TextureFactory.
  3. Applies per-category texture settings:
       - ui-icons / upgrade-icons: LODGroup = UI, CompressionSettings =
         TC_EditorIcon (UserInterface2D), MipGenSettings = NoMipmaps,
         SRGB = True.
       - keyart: LODGroup = UI, CompressionSettings = TC_EditorIcon,
         MipGenSettings = NoMipmaps (single large marketing image, not
         tiled/mipped in-game).
       - characters / buildings / vfx: default (TC_Default), mipmaps ON
         (sprites may be scaled at various distances in 3D).
       - environment: TC_Default, mipmaps ON, TilingMethod-friendly
         (no special wrap override needed; UE defaults to Wrap addressing
         which is what a tileable ground texture wants).
  4. Creates (or reuses) a simple unlit sprite Material Instance hook per
     category under /Game/Emberbound/Materials/Generated/MI_<Category>_Base
     so gameplay code can just set the texture param without hand-wiring
     materials per asset. (Documented here; NOT executed by this worker.)
  5. Saves all created/modified packages and prints IMPORT_OK on success,
     or a per-asset error list without aborting the whole batch.

Idempotent: re-running skips assets that already exist at their ue_path
unless FORCE_REIMPORT is set True below.
"""

from __future__ import annotations

import json
import os

# NOTE: this import only succeeds inside UnrealEditor's embedded Python.
import unreal  # noqa: E402  pylint: disable=import-error

REPO_ROOT = r"E:\mobilegamer\EmberboundIsekai"
MANIFEST_JSON_PATH = os.path.join(REPO_ROOT, "Tools", "AssetGeneration", "asset_manifest.json")

FORCE_REIMPORT = False

# Base material per category -- created once, reused as a parent for
# per-asset Material Instances. All are simple unlit/masked sprite
# materials; gameplay/UI code binds the "Tex" texture parameter.
MATERIAL_PARENT_PATH = "/Game/Emberbound/Materials/Generated/M_GeneratedSpriteBase"

CATEGORY_TEXTURE_SETTINGS = {
    "ui-icons": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_UI,
                      compression=unreal.TextureCompressionSettings.TC_EDITOR_ICON,
                      mips=unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS),
    "upgrade-icons": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_UI,
                           compression=unreal.TextureCompressionSettings.TC_EDITOR_ICON,
                           mips=unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS),
    "keyart": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_UI,
                    compression=unreal.TextureCompressionSettings.TC_EDITOR_ICON,
                    mips=unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS),
    "characters": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_CHARACTER,
                        compression=unreal.TextureCompressionSettings.TC_DEFAULT,
                        mips=unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP),
    "buildings": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_WORLD,
                       compression=unreal.TextureCompressionSettings.TC_DEFAULT,
                       mips=unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP),
    "vfx": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_EFFECTS,
                compression=unreal.TextureCompressionSettings.TC_DEFAULT,
                mips=unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP),
    "environment": dict(lod_group=unreal.TextureGroup.TEXTUREGROUP_WORLD,
                         compression=unreal.TextureCompressionSettings.TC_DEFAULT,
                         mips=unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP),
}


def _largest_processed_file(entry: dict) -> str | None:
    """Pick the biggest-resolution processed PNG (by filename size token) for a manifest entry."""
    files = entry.get("processed_files", [])
    if not files:
        return None

    def size_key(path: str) -> int:
        stem = os.path.splitext(os.path.basename(path))[0]
        token = stem.split("_")[-1]
        if "x" in token:
            try:
                w, h = token.split("x")
                return int(w) * int(h)
            except ValueError:
                return 0
        try:
            v = int(token)
            return v * v
        except ValueError:
            return 0

    best = max(files, key=size_key)
    return os.path.join(REPO_ROOT, best.replace("/", os.sep))


def _import_texture(png_abs_path: str, ue_path: str) -> unreal.Texture2D | None:
    dest_path, asset_name = ue_path.rsplit("/", 1)

    if not FORCE_REIMPORT and unreal.EditorAssetLibrary.does_asset_exist(ue_path):
        unreal.log(f"[import_to_unreal] SKIP (exists): {ue_path}")
        return unreal.load_asset(ue_path)

    task = unreal.AssetImportTask()
    task.filename = png_abs_path
    task.destination_path = dest_path
    task.destination_name = asset_name
    task.automated = True
    task.save = True
    task.replace_existing = FORCE_REIMPORT

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])

    imported = task.get_editor_property("imported_object_paths")
    if not imported:
        unreal.log_error(f"[import_to_unreal] FAILED to import {png_abs_path} -> {ue_path}")
        return None
    return unreal.load_asset(imported[0])


def _apply_texture_settings(tex: unreal.Texture2D, category: str) -> None:
    settings = CATEGORY_TEXTURE_SETTINGS.get(category)
    if not settings or tex is None:
        return
    tex.set_editor_property("lod_group", settings["lod_group"])
    tex.set_editor_property("compression_settings", settings["compression"])
    tex.set_editor_property("mip_gen_settings", settings["mips"])
    tex.set_editor_property("srgb", True)
    tex.modify()
    tex.post_edit_change()


def _ensure_material_instance(tex: unreal.Texture2D, category: str, asset_id: str) -> None:
    """Create/refresh MI_<Category>_<AssetId> parented to the shared sprite base material,
    with the Tex parameter bound. Parent material creation is NOT handled here -- it's
    expected to already exist (created by an earlier editor content script) at
    MATERIAL_PARENT_PATH; if missing, this step is skipped with a warning so texture
    import still succeeds."""
    parent = unreal.load_asset(MATERIAL_PARENT_PATH)
    if parent is None:
        unreal.log_warning(
            f"[import_to_unreal] Parent material {MATERIAL_PARENT_PATH} not found; "
            f"skipping material instance for {asset_id} (texture import still OK)."
        )
        return

    category_path = f"/Game/Emberbound/Materials/Generated/{category}"
    mi_name = f"MI_{asset_id.replace('-', '_')}"
    mi_path = f"{category_path}/{mi_name}"

    if not FORCE_REIMPORT and unreal.EditorAssetLibrary.does_asset_exist(mi_path):
        return

    factory = unreal.MaterialInstanceConstantFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    mi = asset_tools.create_asset(mi_name, category_path, unreal.MaterialInstanceConstant, factory)
    if mi is None:
        return
    unreal.MaterialEditingLibrary.set_material_instance_parent(mi, parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(mi, "Tex", tex)
    unreal.EditorAssetLibrary.save_loaded_asset(mi)


def main() -> None:
    if not os.path.exists(MANIFEST_JSON_PATH):
        unreal.log_error(f"[import_to_unreal] manifest not found: {MANIFEST_JSON_PATH}")
        return

    with open(MANIFEST_JSON_PATH, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    assets = manifest.get("assets", {})
    imported, skipped, failed = [], [], []

    for asset_id, entry in assets.items():
        status = entry.get("status")
        if status not in ("generated", "fallback"):
            skipped.append(asset_id)
            continue

        png_path = _largest_processed_file(entry)
        if not png_path or not os.path.exists(png_path):
            unreal.log_error(f"[import_to_unreal] no processed file for {asset_id}")
            failed.append(asset_id)
            continue

        ue_path = entry["ue_path"]
        try:
            tex = _import_texture(png_path, ue_path)
            if tex is None:
                failed.append(asset_id)
                continue
            _apply_texture_settings(tex, entry["category"])
            _ensure_material_instance(tex, entry["category"], asset_id)
            imported.append(asset_id)
        except Exception as exc:  # noqa: BLE001 - keep the batch going
            unreal.log_error(f"[import_to_unreal] EXCEPTION importing {asset_id}: {exc}")
            failed.append(asset_id)

    unreal.EditorAssetLibrary.save_directory("/Game/Emberbound/Generated", False, True)
    unreal.EditorAssetLibrary.save_directory("/Game/Emberbound/Materials/Generated", False, True)

    unreal.log(f"[import_to_unreal] imported={len(imported)} skipped={len(skipped)} failed={len(failed)}")
    if failed:
        unreal.log_error(f"[import_to_unreal] FAILED ids: {failed}")
    else:
        unreal.log("IMPORT_OK")


if __name__ == "__main__":
    main()
