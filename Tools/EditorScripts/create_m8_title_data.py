import unreal

DEST = "/Game/Emberbound/Data/Titles"


def make_title(asset_name, title_id, display, description, effect, magnitude, condition, param, debug_only=False):
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    if unreal.EditorAssetLibrary.does_asset_exist(f"{DEST}/{asset_name}"):
        asset = unreal.EditorAssetLibrary.load_asset(f"{DEST}/{asset_name}")
    else:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.TitleData)
        asset = asset_tools.create_asset(asset_name, DEST, unreal.TitleData, factory)
    asset.set_editor_property("id", title_id)
    asset.set_editor_property("display_name", display)
    asset.set_editor_property("description", description)
    passive = unreal.TitlePassiveSpec()
    passive.set_editor_property("effect_type", effect)
    passive.set_editor_property("magnitude", magnitude)
    asset.set_editor_property("passive", passive)
    asset.set_editor_property("unlock_condition", condition)
    asset.set_editor_property("unlock_param", param)
    asset.set_editor_property("debug_only", debug_only)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


def main():
    unreal.EditorAssetLibrary.make_directory(DEST)
    make_title(
        "DA_Title_FormerOfficeDrone",
        "former-office-drone",
        "Former Office Drone",
        "Turns soul-crushing spreadsheet trauma into mildly better passive income.",
        unreal.TitlePassiveEffectType.OFFLINE_PRODUCTION_PERCENT,
        0.05,
        unreal.TitleUnlockCondition.OFFLINE_REWARD_COLLECTED,
        1,
    )
    make_title(
        "DA_Title_TruckKunSurvivor",
        "truck-kun-survivor",
        "Truck-kun Survivor",
        "A suspiciously specific second chance. Debug-only until revive flow exists.",
        unreal.TitlePassiveEffectType.REVIVE_PER_RUN,
        1.0,
        unreal.TitleUnlockCondition.NONE,
        1,
        True,
    )
    make_title(
        "DA_Title_FailedChosenOne",
        "failed-chosen-one",
        "Failed Chosen One",
        "Deals more damage while almost dead, which is technically a strategy.",
        unreal.TitlePassiveEffectType.LOW_HEALTH_DAMAGE_PERCENT,
        0.10,
        unreal.TitleUnlockCondition.BOSS_DEFEATED,
        1,
    )
    make_title(
        "DA_Title_SpreadsheetHero",
        "spreadsheet-hero",
        "Spreadsheet Hero",
        "Finds a rounding error in the shopkeeper's favor and fixes it badly.",
        unreal.TitlePassiveEffectType.UPGRADE_COST_PERCENT,
        -0.05,
        unreal.TitleUnlockCondition.BUILDING_UPGRADES,
        5,
    )
    make_title(
        "DA_Title_GenreSavvyIdiot",
        "genre-savvy-idiot",
        "Genre-Savvy Idiot",
        "Knows every trope except the one currently happening.",
        unreal.TitlePassiveEffectType.XP_GAIN_PERCENT,
        0.05,
        unreal.TitleUnlockCondition.RUN_UPGRADES_PICKED,
        10,
    )
    print("M8_TITLE_CONTENT_OK assets=5")


if __name__ == "__main__":
    main()
