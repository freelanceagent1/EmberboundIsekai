from __future__ import annotations

import json
import os

import unreal


REPO_ROOT = r"E:\mobilegamer\EmberboundIsekai"
MANIFEST_JSON_PATH = os.path.join(REPO_ROOT, "Tools", "AssetGeneration", "asset_manifest.json")
MATERIAL_PATH = "/Game/Emberbound/Materials/Generated"
FORCE_REIMPORT = False


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


UPGRADE_ASSET_NAMES = {
    "bone-shield": "DA_BoneShield",
    "companion-slime": "DA_CompanionSlime",
    "crit-focus": "DA_CritFocus",
    "fire-arrow": "DA_FireArrow",
    "frost-arrow": "DA_FrostArrow",
    "gold-rush": "DA_GoldRush",
    "healing-wisp": "DA_HealingWisp",
    "heavy-arrow": "DA_HeavyArrow",
    "pickup-magnet": "DA_PickupMagnet",
    "piercing-shot": "DA_PiercingShot",
    "poison-arrow": "DA_PoisonArrow",
    "rapid-fire": "DA_RapidFire",
    "ricochet-shot": "DA_RicochetShot",
    "soul-flame-trail": "DA_SoulFlameTrail",
    "split-shot": "DA_SplitShot",
}


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def object_path(package_path: str) -> str:
    return f"{package_path}.{package_path.rsplit('/', 1)[1]}"


def selected_processed_file(entry: dict) -> str | None:
    files = entry.get("processed_files", [])
    if not files:
        return None

    def size_key(path: str) -> int:
        stem = os.path.splitext(os.path.basename(path))[0]
        token = stem.split("_")[-1]
        if "x" in token:
            parts = token.split("x")
            return int(parts[0]) * int(parts[1])
        value = int(token)
        return value * value

    best = max(files, key=size_key)
    if best.endswith("_256.png"):
        return None
    return os.path.join(REPO_ROOT, best.replace("/", os.sep))


def import_texture(png_abs_path: str, package_path: str) -> unreal.Texture2D | None:
    if not FORCE_REIMPORT and unreal.EditorAssetLibrary.does_asset_exist(package_path):
        return unreal.EditorAssetLibrary.load_asset(package_path)

    dest_path, asset_name = package_path.rsplit("/", 1)
    task = unreal.AssetImportTask()
    task.filename = png_abs_path
    task.destination_path = dest_path
    task.destination_name = asset_name
    task.automated = True
    task.save = True
    task.replace_existing = FORCE_REIMPORT
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = task.get_editor_property("imported_object_paths")
    return unreal.load_asset(imported[0]) if imported else None


def apply_texture_settings(tex: unreal.Texture2D, category: str) -> None:
    settings = CATEGORY_TEXTURE_SETTINGS[category]
    tex.set_editor_property("lod_group", settings["lod_group"])
    tex.set_editor_property("compression_settings", settings["compression"])
    tex.set_editor_property("mip_gen_settings", settings["mips"])
    tex.set_editor_property("srgb", True)
    tex.modify()
    unreal.EditorAssetLibrary.save_loaded_asset(tex)


def reset_material(mat: unreal.Material) -> None:
    return


def create_material(name: str, masked: bool = True, translucent: bool = False, unlit: bool = True,
                    two_sided: bool = True, panner: bool = False, decal: bool = False,
                    shadow: bool = False) -> unreal.Material:
    ensure_dir(MATERIAL_PATH)
    asset_path = f"{MATERIAL_PATH}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, MATERIAL_PATH, unreal.Material, unreal.MaterialFactoryNew())

    mat.set_editor_property("two_sided", two_sided)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT if translucent else unreal.BlendMode.BLEND_MASKED if masked else unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT if unlit else unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    if decal:
        mat.set_editor_property("material_domain", unreal.MaterialDomain.MD_DEFERRED_DECAL)

    if shadow:
        color = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -300, 0)
        color.set_editor_property("constant", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
        alpha = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, 160)
        alpha.set_editor_property("r", 0.32)
        unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.MaterialEditingLibrary.connect_material_property(alpha, "", unreal.MaterialProperty.MP_OPACITY)
        unreal.MaterialEditingLibrary.recompile_material(mat)
        unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
        return mat

    tex = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -700, 0)
    tex.set_editor_property("parameter_name", "Tex")
    if panner:
        panner_expr = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionPanner, -950, 80)
        panner_expr.set_editor_property("speed_x", 0.02)
        panner_expr.set_editor_property("speed_y", 0.12)
        unreal.MaterialEditingLibrary.connect_material_expressions(panner_expr, "", tex, "UVs")
    tint = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, 260)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    flash = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, 420)
    flash.set_editor_property("parameter_name", "Flash")
    flash.set_editor_property("default_value", 0.0)
    white = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -470, 420)
    white.set_editor_property("constant", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    lerp = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -260, 300)
    mul = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -40, 80)
    unreal.MaterialEditingLibrary.connect_material_expressions(tint, "", lerp, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(white, "", lerp, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(flash, "", lerp, "Alpha")
    unreal.MaterialEditingLibrary.connect_material_expressions(tex, "RGB", mul, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(lerp, "", mul, "B")
    unreal.MaterialEditingLibrary.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR if unlit else unreal.MaterialProperty.MP_BASE_COLOR)
    alpha_property = unreal.MaterialProperty.MP_OPACITY if translucent else unreal.MaterialProperty.MP_OPACITY_MASK
    unreal.MaterialEditingLibrary.connect_material_property(tex, "A", alpha_property)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
    return mat


def create_materials() -> None:
    create_material("M_Generated_Icon", masked=False, translucent=True, unlit=True, two_sided=False)
    create_material("M_Generated_Billboard", masked=True, translucent=False, unlit=True, two_sided=True)
    create_material("M_Generated_Floor", masked=False, translucent=False, unlit=False, two_sided=False)
    create_material("M_Stylized_Enemy", masked=True, translucent=False, unlit=True, two_sided=True)
    create_material("M_Campfire_Flame", masked=False, translucent=True, unlit=True, two_sided=True, panner=True)
    create_material("M_Corruption_Decal", masked=False, translucent=True, unlit=True, two_sided=False, decal=True)
    create_material("M_BlobShadow", masked=False, translucent=True, unlit=True, two_sided=True, shadow=True)
    create_floor_instance("MI_ExpeditionFloor_CorruptedGrass", "/Game/Emberbound/Generated/Environment/T_CorruptedGrass")
    create_floor_instance("MI_CampFloor_MeadowGrass", "/Game/Emberbound/Generated/Environment/T_MeadowGrass")
    create_floor_instance("MI_CampDirt", "/Game/Emberbound/Generated/Environment/T_CampDirt")


def create_floor_instance(name: str, texture_path: str) -> None:
    asset_path = f"{MATERIAL_PATH}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
    parent = unreal.EditorAssetLibrary.load_asset(f"{MATERIAL_PATH}/M_Generated_Floor")
    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    factory = unreal.MaterialInstanceConstantFactoryNew()
    mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, MATERIAL_PATH, unreal.MaterialInstanceConstant, factory)
    unreal.MaterialEditingLibrary.set_material_instance_parent(mi, parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(mi, "Tex", texture)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi, "Flash", 0.0)
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)


def assign_upgrade_icons(manifest_assets: dict) -> int:
    assigned = 0
    for asset_id, asset_name in UPGRADE_ASSET_NAMES.items():
        entry = manifest_assets.get(asset_id)
        if not entry:
            continue
        upgrade_path = f"/Game/Emberbound/Data/Upgrades/{asset_name}"
        upgrade = unreal.EditorAssetLibrary.load_asset(upgrade_path)
        tex = unreal.EditorAssetLibrary.load_asset(entry["ue_path"])
        if upgrade and tex:
            upgrade.set_editor_property("icon", tex)
            unreal.EditorAssetLibrary.save_loaded_asset(upgrade)
            assigned += 1
    return assigned


def verify_manifest(manifest_assets: dict) -> None:
    missing = []
    for asset_id, entry in manifest_assets.items():
        if entry.get("status") in ("generated", "fallback") and not unreal.EditorAssetLibrary.does_asset_exist(entry["ue_path"]):
            missing.append(f"{asset_id}:{entry['ue_path']}")
    material_names = [
        "M_Generated_Icon",
        "M_Generated_Billboard",
        "M_Generated_Floor",
        "M_Stylized_Enemy",
        "M_Campfire_Flame",
        "M_Corruption_Decal",
        "M_BlobShadow",
        "MI_ExpeditionFloor_CorruptedGrass",
        "MI_CampFloor_MeadowGrass",
        "MI_CampDirt",
    ]
    for name in material_names:
        path = f"{MATERIAL_PATH}/{name}"
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            missing.append(path)
    if missing:
        raise RuntimeError(f"Missing generated M5b assets: {missing}")


def main() -> None:
    with open(MANIFEST_JSON_PATH, "r", encoding="utf-8") as f:
        manifest = json.load(f)
    assets = manifest.get("assets", {})

    imported = 0
    failed = []
    for asset_id, entry in assets.items():
        if entry.get("status") not in ("generated", "fallback"):
            continue
        png_path = selected_processed_file(entry)
        if not png_path or not os.path.exists(png_path):
            failed.append(asset_id)
            continue
        tex = import_texture(png_path, entry["ue_path"])
        if tex is None:
            failed.append(asset_id)
            continue
        apply_texture_settings(tex, entry["category"])
        imported += 1

    if failed:
        raise RuntimeError(f"Import failures: {failed}")

    create_materials()
    assigned = assign_upgrade_icons(assets)
    verify_manifest(assets)
    unreal.EditorAssetLibrary.save_directory("/Game/Emberbound/Generated", only_if_is_dirty=False, recursive=True)
    unreal.EditorAssetLibrary.save_directory(MATERIAL_PATH, only_if_is_dirty=False, recursive=True)
    unreal.log_warning(f"UPGRADE_ICONS_ASSIGNED n={assigned}")
    unreal.log_warning(f"IMPORT_OK n={imported}")


if __name__ == "__main__":
    main()
