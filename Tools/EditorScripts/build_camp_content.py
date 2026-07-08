import math
import unreal


MAP_PATH = "/Game/Emberbound/Maps/Camp/L_Camp"
DATA_PATH = "/Game/Emberbound/Data/Buildings"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def save_asset(path):
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)


def make_cost(**amounts):
    cost = unreal.ResourceCost()
    values = {}
    for key, amount in amounts.items():
        values[getattr(unreal.EmberResource, key)] = int(amount)
    cost.set_editor_property("amounts", values)
    return cost


def scaled_cost(base):
    result = []
    for level in range(1, 5):
        scale = math.pow(1.8, level - 1)
        result.append({name: max(1, int(round(amount * scale))) for name, amount in base.items()})
    return result


def create_building(asset_name, building_id, display_name, description, base_cost, effect):
    ensure_dir(DATA_PATH)
    asset_path = f"{DATA_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        data = unreal.EditorAssetLibrary.load_asset(asset_path)
    else:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.BuildingData)
        data = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, DATA_PATH, unreal.BuildingData, factory)
    data.set_editor_property("id", building_id)
    data.set_editor_property("display_name", display_name)
    data.set_editor_property("description", description)
    data.set_editor_property("max_level", 5)
    data.set_editor_property("upgrade_costs", [make_cost(**cost) for cost in scaled_cost(base_cost)])
    data.set_editor_property("per_level_effect_magnitude", [float(effect * level) for level in range(1, 6)])
    save_asset(asset_path)


def build_map():
    ensure_dir("/Game/Emberbound/Maps/Camp")
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        registry.scan_paths_synchronous(["/Engine/Maps", "/Engine/Maps/Templates"], True)
        sources = [
            "/Engine/Maps/Templates/Template_Default",
            "/Engine/Maps/Templates/OpenWorld",
            "/Engine/Maps/Entry",
        ]
        source = next((path for path in sources if unreal.EditorAssetLibrary.does_asset_exist(path)), None)
        if not source or not unreal.EditorAssetLibrary.duplicate_asset(source, MAP_PATH):
            raise RuntimeError("Failed to duplicate an engine map to L_Camp")
    save_asset(MAP_PATH)


def main():
    create_building("DA_CampfireCore", "campfire-core", "Campfire Core",
                    "Offline cap 4h per level plus safe radius. Level 2 unlocks Monster Pen.",
                    {"GOLD": 20, "WOOD": 5}, 4.0)
    create_building("DA_GrimForge", "grim-forge", "Grim Forge",
                    "Increases player damage by 15% per level.",
                    {"GOLD": 15, "SOUL_ASH": 2}, 15.0)
    create_building("DA_MonsterPen", "monster-pen", "Monster Pen",
                    "Adds one monster assignment slot per level.",
                    {"GOLD": 25, "WOOD": 5}, 1.0)
    create_building("DA_Watchtower", "watchtower", "Watchtower",
                    "Defense event placeholder. Defender damage +10% per level.",
                    {"GOLD": 20, "WOOD": 3}, 10.0)
    build_map()
    unreal.EditorAssetLibrary.save_directory(DATA_PATH, only_if_is_dirty=False, recursive=True)
    unreal.EditorAssetLibrary.save_directory("/Game/Emberbound/Maps/Camp", only_if_is_dirty=False, recursive=True)
    unreal.log_warning("CONTENT_OK")


if __name__ == "__main__":
    main()
