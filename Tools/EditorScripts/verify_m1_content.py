import unreal

required = [
    "/Game/Emberbound/Maps/Expedition/L_Expedition",
    "/Game/Emberbound/Materials/Stylized/M_Enemy",
    "/Game/Emberbound/Materials/Stylized/M_Floor",
    "/Game/Emberbound/Materials/Stylized/M_Projectile",
    "/Game/Emberbound/Data/Enemies/GreenSlime",
    "/Game/Emberbound/Data/Enemies/RedSlime",
    "/Game/Emberbound/Data/Enemies/GoblinStabber",
    "/Game/Emberbound/Data/Enemies/GoblinArcher",
    "/Game/Emberbound/Data/Enemies/BoneRat",
]

missing = [path for path in required if not unreal.EditorAssetLibrary.does_asset_exist(path)]
if missing:
    raise RuntimeError("Missing required M1 assets: " + ", ".join(missing))

print("M1_ASSET_VERIFY_OK")
