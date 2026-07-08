import unreal


REQUIRED_ASSETS = [
    "/Game/Emberbound/Maps/Camp/L_Camp",
    "/Game/Emberbound/Data/Buildings/DA_CampfireCore",
    "/Game/Emberbound/Data/Buildings/DA_GrimForge",
    "/Game/Emberbound/Data/Buildings/DA_MonsterPen",
    "/Game/Emberbound/Data/Buildings/DA_Watchtower",
]


def main():
    missing = [path for path in REQUIRED_ASSETS if not unreal.EditorAssetLibrary.does_asset_exist(path)]
    if missing:
        raise RuntimeError(f"Missing M4 assets: {missing}")
    ids = set()
    for path in REQUIRED_ASSETS[1:]:
        data = unreal.EditorAssetLibrary.load_asset(path)
        ids.add(str(data.get_editor_property("id")))
        if data.get_editor_property("max_level") != 5:
            raise RuntimeError(f"{path} max_level is not 5")
        if len(data.get_editor_property("upgrade_costs")) != 4:
            raise RuntimeError(f"{path} does not have 4 upgrade costs")
    expected = {"campfire-core", "grim-forge", "monster-pen", "watchtower"}
    if ids != expected:
        raise RuntimeError(f"Unexpected building ids: {ids}")
    unreal.log_warning("M4_ASSET_VERIFY_OK")


if __name__ == "__main__":
    main()
