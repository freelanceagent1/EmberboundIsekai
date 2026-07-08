import os
import unreal

PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
SOURCE_DIR = os.path.join(PROJECT_DIR, "Saved", "GeneratedSFX")
DEST = "/Game/Emberbound/Generated/SFX"


def main():
    if not os.path.isdir(SOURCE_DIR):
        raise RuntimeError(f"Missing generated SFX dir: {SOURCE_DIR}")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    tasks = []
    for filename in sorted(os.listdir(SOURCE_DIR)):
        if not filename.lower().endswith(".wav"):
            continue
        task = unreal.AssetImportTask()
        task.filename = os.path.join(SOURCE_DIR, filename)
        task.destination_path = DEST
        task.automated = True
        task.replace_existing = True
        task.save = True
        tasks.append(task)

    asset_tools.import_asset_tasks(tasks)
    imported = [task.imported_object_paths for task in tasks]
    if len(tasks) != 11:
        raise RuntimeError(f"Expected 11 wav imports, got {len(tasks)}: {imported}")
    unreal.EditorAssetLibrary.save_directory(DEST)
    print(f"IMPORT_OK n={len(tasks)} dest={DEST}")


if __name__ == "__main__":
    main()
