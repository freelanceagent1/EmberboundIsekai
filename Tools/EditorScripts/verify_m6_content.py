import unreal


DATA = {
    "/Game/Emberbound/Data/Monsters/DA_Slime": ("slime", 5),
    "/Game/Emberbound/Data/Monsters/DA_BoneRat": ("bone-rat", 5),
    "/Game/Emberbound/Data/Monsters/DA_SkeletonArcher": ("skeleton-archer", 8),
}


def main():
    missing = []
    bad = []
    for path, (monster_id, shards) in DATA.items():
        data = unreal.EditorAssetLibrary.load_asset(path)
        if not data:
            missing.append(path)
            continue
        if str(data.get_editor_property("id")) != monster_id:
            bad.append(f"{path}:id")
        if int(data.get_editor_property("shards_to_unlock")) != shards:
            bad.append(f"{path}:shards")
        if not data.get_editor_property("icon_texture"):
            bad.append(f"{path}:icon")
        if not data.get_editor_property("billboard_texture"):
            bad.append(f"{path}:billboard")
    if missing or bad:
        raise RuntimeError(f"M6 verify failed missing={missing} bad={bad}")
    unreal.log_warning("M6_ASSET_VERIFY_OK monsters=3")


if __name__ == "__main__":
    main()
