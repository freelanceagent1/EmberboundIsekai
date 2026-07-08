import json
import os

import unreal


REPO_ROOT = r"E:\mobilegamer\EmberboundIsekai"
MANIFEST_JSON_PATH = os.path.join(REPO_ROOT, "Tools", "AssetGeneration", "asset_manifest.json")

REQUIRED_MATERIALS = [
    "/Game/Emberbound/Materials/Generated/M_Generated_Icon",
    "/Game/Emberbound/Materials/Generated/M_Generated_Billboard",
    "/Game/Emberbound/Materials/Generated/M_Generated_Floor",
    "/Game/Emberbound/Materials/Generated/M_Stylized_Enemy",
    "/Game/Emberbound/Materials/Generated/M_Campfire_Flame",
    "/Game/Emberbound/Materials/Generated/M_Corruption_Decal",
    "/Game/Emberbound/Materials/Generated/M_BlobShadow",
    "/Game/Emberbound/Materials/Generated/MI_ExpeditionFloor_CorruptedGrass",
    "/Game/Emberbound/Materials/Generated/MI_CampFloor_MeadowGrass",
    "/Game/Emberbound/Materials/Generated/MI_CampDirt",
]

UPGRADE_ASSET_NAMES = [
    "DA_BoneShield",
    "DA_CompanionSlime",
    "DA_CritFocus",
    "DA_FireArrow",
    "DA_FrostArrow",
    "DA_GoldRush",
    "DA_HealingWisp",
    "DA_HeavyArrow",
    "DA_PickupMagnet",
    "DA_PiercingShot",
    "DA_PoisonArrow",
    "DA_RapidFire",
    "DA_RicochetShot",
    "DA_SoulFlameTrail",
    "DA_SplitShot",
]


def main():
    with open(MANIFEST_JSON_PATH, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    missing = []
    expected = 0
    for asset_id, entry in manifest.get("assets", {}).items():
        if entry.get("status") in ("generated", "fallback"):
            expected += 1
            if not unreal.EditorAssetLibrary.does_asset_exist(entry["ue_path"]):
                missing.append(f"{asset_id}:{entry['ue_path']}")

    for path in REQUIRED_MATERIALS:
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            missing.append(path)

    unresolved_icons = []
    for name in UPGRADE_ASSET_NAMES:
        path = f"/Game/Emberbound/Data/Upgrades/{name}"
        data = unreal.EditorAssetLibrary.load_asset(path)
        if not data:
            missing.append(path)
            continue
        icon = data.get_editor_property("icon")
        if not icon:
            unresolved_icons.append(name)

    if missing or unresolved_icons:
        raise RuntimeError(f"M5b verify failed missing={missing} unresolved_icons={unresolved_icons}")

    unreal.log_warning(f"M5B_ASSET_VERIFY_OK assets={expected} materials={len(REQUIRED_MATERIALS)} upgrade_icons={len(UPGRADE_ASSET_NAMES)}")


if __name__ == "__main__":
    main()
