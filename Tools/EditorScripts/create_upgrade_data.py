import unreal


UPGRADES = [
    ("split-shot", "Split Shot", "Fire an extra angled arrow.", "COMMON", 3, "SPLIT_SHOT", {"count": 1.0}),
    ("piercing-shot", "Piercing Shot", "Arrows pass through one more enemy.", "COMMON", 3, "PIERCING_SHOT", {"pierce": 1.0}),
    ("ricochet-shot", "Ricochet Shot", "Arrows bounce to a nearby enemy.", "RARE", 2, "RICOCHET_SHOT", {"ricochet": 1.0}),
    ("rapid-fire", "Rapid Fire", "Shoot 15% faster.", "COMMON", 5, "RAPID_FIRE", {"multiplier": 0.85}),
    ("heavy-arrow", "Heavy Arrow", "Arrows deal 25% more damage.", "COMMON", 5, "HEAVY_ARROW", {"multiplier": 1.25}),
    ("fire-arrow", "Fire Arrow", "Projectile hits burn for 4 DPS over 3 seconds.", "RARE", 1, "FIRE_ARROW", {"dps": 4.0, "duration": 3.0}),
    ("poison-arrow", "Poison Arrow", "Projectile hits poison for 2 DPS over 6 seconds. Stacks.", "RARE", 3, "POISON_ARROW", {"dps": 2.0, "duration": 6.0}),
    ("frost-arrow", "Frost Arrow", "Projectile hits slow enemies by 30% for 2 seconds.", "RARE", 1, "FROST_ARROW", {"slow": 0.3, "duration": 2.0}),
    ("bone-shield", "Bone Shield", "A circling bone damages enemies on touch.", "COMMON", 3, "BONE_SHIELD", {}),
    ("healing-wisp", "Healing Wisp", "Restore 2 HP every 3 seconds.", "COMMON", 1, "HEALING_WISP", {}),
    ("pickup-magnet", "Pickup Magnet", "Increase pickup radius by 50%.", "COMMON", 3, "PICKUP_MAGNET", {"multiplier": 1.5}),
    ("crit-focus", "Crit Focus", "Gain 8% critical chance.", "RARE", 4, "CRIT_FOCUS", {"crit": 0.08}),
    ("companion-slime", "Companion Slime", "Summon a slime that fires acid blobs.", "EPIC", 1, "COMPANION_SLIME", {}),
    ("soul-flame-trail", "Soul Flame Trail", "Leave short-lived flame patches while moving.", "EPIC", 1, "SOUL_FLAME_TRAIL", {}),
    ("gold-rush", "Gold Rush", "Gain 25% more gold during the run.", "RARE", 4, "GOLD_RUSH", {"bonus": 0.25}),
]


def enum_value(enum_name, value_name):
    enum = getattr(unreal, enum_name)
    return getattr(enum, value_name)


def asset_name(upgrade_id):
    return "DA_" + "".join(part.capitalize() for part in upgrade_id.split("-"))


def main():
    package_path = "/Game/Emberbound/Data/Upgrades"
    unreal.EditorAssetLibrary.make_directory(package_path)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.RunUpgradeData)

    for upgrade_id, title, desc, rarity, max_stacks, effect, params in UPGRADES:
        name = asset_name(upgrade_id)
        asset_path = f"{package_path}/{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            data = unreal.EditorAssetLibrary.load_asset(asset_path)
        else:
            data = tools.create_asset(name, package_path, unreal.RunUpgradeData, factory)
        data.set_editor_property("id", upgrade_id)
        data.set_editor_property("title", unreal.Text(title))
        data.set_editor_property("description", unreal.Text(desc))
        data.set_editor_property("rarity", enum_value("RunUpgradeRarity", rarity))
        data.set_editor_property("max_stacks", max_stacks)
        data.set_editor_property("effect_type", enum_value("RunUpgradeEffectType", effect))
        data.set_editor_property("float_params", params)
        unreal.EditorAssetLibrary.save_loaded_asset(data)

    missing = []
    for upgrade_id, *_ in UPGRADES:
        path = f"{package_path}/{asset_name(upgrade_id)}"
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            missing.append(path)
    if missing:
        raise RuntimeError("Missing upgrade assets: " + ", ".join(missing))
    print("CONTENT_OK")


if __name__ == "__main__":
    main()
