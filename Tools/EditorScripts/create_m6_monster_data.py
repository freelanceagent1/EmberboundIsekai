import unreal


DATA_PATH = "/Game/Emberbound/Data/Monsters"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def texture(path):
    return unreal.EditorAssetLibrary.load_asset(path)


def output(resource_name, per_minute):
    item = unreal.MonsterWorkerOutput()
    item.set_editor_property("resource", getattr(unreal.EmberResource, resource_name))
    item.set_editor_property("per_minute", float(per_minute))
    return item


def role_mask(*roles):
    values = {
        "COMPANION": int(unreal.MonsterRoleFlags.COMPANION.value),
        "WORKER": int(unreal.MonsterRoleFlags.WORKER.value),
        "DEFENDER": int(unreal.MonsterRoleFlags.DEFENDER.value),
    }
    mask = 0
    for role in roles:
        mask |= values[role]
    return mask


def create_monster(asset_name, monster_id, display_name, description, shards, roles, worker_outputs, companion_damage, companion_interval, defender_dps, texture_path):
    ensure_dir(DATA_PATH)
    asset_path = f"{DATA_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        data = unreal.EditorAssetLibrary.load_asset(asset_path)
    else:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.MonsterData)
        data = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, DATA_PATH, unreal.MonsterData, factory)
    data.set_editor_property("id", monster_id)
    data.set_editor_property("display_name", display_name)
    data.set_editor_property("description", description)
    data.set_editor_property("shards_to_unlock", int(shards))
    data.set_editor_property("allowed_roles", role_mask(*roles))
    data.set_editor_property("worker_outputs", worker_outputs)
    data.set_editor_property("companion_damage", float(companion_damage))
    data.set_editor_property("companion_attack_interval", float(companion_interval))
    data.set_editor_property("companion_projectile_speed", 1000.0)
    data.set_editor_property("defender_dps", float(defender_dps))
    data.set_editor_property("icon_texture", texture(texture_path))
    data.set_editor_property("billboard_texture", texture(texture_path))
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)


def main():
    create_monster(
        "DA_Slime",
        "slime",
        "Slime",
        "A loyal acid-spitting camp helper.",
        5,
        ["COMPANION", "WORKER"],
        [output("GOLD", 2.0)],
        6.0,
        1.2,
        0.0,
        "/Game/Emberbound/Generated/Characters/T_SlimeCompanion.T_SlimeCompanion",
    )
    create_monster(
        "DA_BoneRat",
        "bone-rat",
        "Bone Rat",
        "A skittering worker that also watches the camp perimeter.",
        5,
        ["WORKER", "DEFENDER"],
        [output("GOLD", 1.0), output("SOUL_ASH", 0.5)],
        0.0,
        1.4,
        4.0,
        "/Game/Emberbound/Generated/Characters/T_BoneRat.T_BoneRat",
    )
    create_monster(
        "DA_SkeletonArcher",
        "skeleton-archer",
        "Skeleton Archer",
        "A brittle marksman suited to expedition support or defense duty.",
        8,
        ["COMPANION", "DEFENDER"],
        [],
        8.0,
        1.2,
        7.0,
        "/Game/Emberbound/Generated/Characters/T_SkeletonArcher.T_SkeletonArcher",
    )
    unreal.EditorAssetLibrary.save_directory(DATA_PATH, only_if_is_dirty=False, recursive=True)
    unreal.log_warning("M6_MONSTER_CONTENT_OK assets=3")


if __name__ == "__main__":
    main()
