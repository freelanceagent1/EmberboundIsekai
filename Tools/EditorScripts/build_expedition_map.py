import math
import unreal


MAP_PATH = "/Game/Emberbound/Maps/Expedition/L_Expedition"
MAT_PATH = "/Game/Emberbound/Materials/Stylized"
DATA_PATH = "/Game/Emberbound/Data/Enemies"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def save_asset(path):
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)


def create_material(name, default_tint):
    ensure_dir(MAT_PATH)
    asset_path = f"{MAT_PATH}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        mat = unreal.EditorAssetLibrary.load_asset(asset_path)
    else:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, MAT_PATH, unreal.Material, unreal.MaterialFactoryNew()
        )

    mat.set_editor_property("use_material_attributes", False)
    tint = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -600, 0
    )
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", default_tint)

    flash = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionScalarParameter, -600, 180
    )
    flash.set_editor_property("parameter_name", "Flash")
    flash.set_editor_property("default_value", 0.0)

    white = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -350, 120
    )
    white.set_editor_property("constant", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    lerp = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionLinearInterpolate, -150, 0
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(tint, "", lerp, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(white, "", lerp, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(flash, "", lerp, "Alpha")
    unreal.MaterialEditingLibrary.connect_material_property(
        lerp, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.recompile_material(mat)
    save_asset(asset_path)
    return mat


def create_enemy_data(name, display, hp, damage, speed, ranged, xp, tint, scale, drops):
    ensure_dir(DATA_PATH)
    asset_path = f"{DATA_PATH}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        data = unreal.EditorAssetLibrary.load_asset(asset_path)
    else:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.EmberEnemyData)
        data = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, DATA_PATH, unreal.EmberEnemyData, factory
        )

    data.set_editor_property("display_name", display)
    data.set_editor_property("max_hp", float(hp))
    data.set_editor_property("contact_damage", float(damage))
    data.set_editor_property("move_speed", float(speed))
    data.set_editor_property(
        "attack_type",
        unreal.EmberEnemyAttackType.RANGED if ranged else unreal.EmberEnemyAttackType.MELEE,
    )
    data.set_editor_property("projectile_damage", 9.0 if ranged else 0.0)
    data.set_editor_property("projectile_speed", 900.0)
    data.set_editor_property("projectile_interval", 1.6)
    data.set_editor_property("xp_reward", int(xp))
    data.set_editor_property("color_tint", tint)
    data.set_editor_property("visual_scale", scale)
    data.set_editor_property("resource_drops", drops)
    save_asset(asset_path)


def drop(resource, probability, min_amount, max_amount):
    return unreal.EnemyResourceDrop(
        resource=resource,
        probability=probability,
        min_amount=min_amount,
        max_amount=max_amount,
    )


def spawn_static_mesh(asset_path, location, scale, name):
    mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor = actor_subsystem.spawn_actor_from_object(mesh, location)
    actor.set_actor_label(name)
    actor.set_actor_scale3d(scale)
    return actor


def build_map():
    ensure_dir("/Game/Emberbound/Maps/Expedition")
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
            raise RuntimeError("Failed to duplicate an engine map to L_Expedition")
    save_asset(MAP_PATH)


def main():
    create_material("M_Enemy", unreal.LinearColor(0.2, 0.9, 0.2, 1.0))
    create_material("M_Floor", unreal.LinearColor(0.05, 0.18, 0.08, 1.0))
    create_material("M_Projectile", unreal.LinearColor(1.0, 0.72, 0.22, 1.0))

    gold = unreal.EmberResource.GOLD
    wood = unreal.EmberResource.WOOD
    create_enemy_data("GreenSlime", "Green Slime", 18, 8, 220, False, 3,
                      unreal.LinearColor(0.15, 0.9, 0.2, 1), unreal.Vector(0.65, 0.65, 0.45),
                      [drop(gold, 0.40, 1, 2)])
    create_enemy_data("RedSlime", "Red Slime", 14, 10, 340, False, 3,
                      unreal.LinearColor(1.0, 0.12, 0.08, 1), unreal.Vector(0.55, 0.55, 0.4),
                      [drop(gold, 0.40, 1, 2)])
    create_enemy_data("GoblinStabber", "Goblin Stabber", 30, 14, 300, False, 5,
                      unreal.LinearColor(0.2, 0.75, 0.55, 1), unreal.Vector(0.75, 0.75, 0.9),
                      [drop(gold, 0.55, 1, 3), drop(wood, 0.05, 1, 1)])
    create_enemy_data("GoblinArcher", "Goblin Archer", 24, 9, 260, True, 5,
                      unreal.LinearColor(0.75, 0.55, 0.2, 1), unreal.Vector(0.7, 0.7, 0.85),
                      [drop(gold, 0.50, 1, 3)])
    create_enemy_data("BoneRat", "Bone Rat", 10, 6, 420, False, 2,
                      unreal.LinearColor(0.78, 0.78, 0.72, 1), unreal.Vector(0.42, 0.65, 0.32),
                      [])

    build_map()
    unreal.EditorAssetLibrary.save_directory("/Game/Emberbound", only_if_is_dirty=False, recursive=True)
    print("CONTENT_OK")


if __name__ == "__main__":
    main()
