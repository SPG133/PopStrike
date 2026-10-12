"""Run manually in Unreal Editor after building PopStrikeEditor.

Blueprint factories invoke the compiler internally. This script is intentionally
user-run. It only creates missing project Blueprint shells; graphs, defaults and
map wiring are a separate authoring step, not a completed gameplay setup.
"""

import unreal


def require_class(path):
    result = unreal.load_class(None, path)
    if result is None:
        raise RuntimeError(f"Build and reopen the editor first: missing {path}")
    return result


def main():
    specs = (
        ("BP_FPSCharacter", "/Script/PopStrike.PSFPSCharacter"),
        ("BP_MatchPlayerController", "/Script/PopStrike.PSMatchPlayerController"),
        ("BP_MatchGameMode", "/Script/PopStrike.PSMatchGameMode"),
        ("BP_Rifle", "/Script/PopStrike.PSWeapon"),
    )
    parents = {name: require_class(path) for name, path in specs}
    anim_parent = require_class("/Script/PopStrike.PSAnimInstance")
    skeleton = unreal.load_asset("/Game/Characters/Heroes/Mannequin/Meshes/SK_Mannequin")
    if not isinstance(skeleton, unreal.Skeleton):
        raise RuntimeError("The imported mannequin skeleton is missing.")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    folder = "/Game/PopStrike/Blueprints"
    for name, parent in parents.items():
        path = f"{folder}/{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            unreal.log(f"Preserved existing asset: {path}")
            continue
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent)
        blueprint = asset_tools.create_asset(name, folder, unreal.Blueprint, factory)
        if blueprint is None or not unreal.EditorAssetLibrary.save_loaded_asset(blueprint):
            raise RuntimeError(f"Could not create/save {path}")
        unreal.log(f"Created Blueprint shell: {path}")

    anim_path = "/Game/PopStrike/Animations/ABP_FPSCharacter"
    if not unreal.EditorAssetLibrary.does_asset_exist(anim_path):
        factory = unreal.AnimBlueprintFactory()
        factory.set_editor_property("parent_class", anim_parent)
        factory.set_editor_property("target_skeleton", skeleton)
        blueprint = asset_tools.create_asset(
            "ABP_FPSCharacter", "/Game/PopStrike/Animations", unreal.AnimBlueprint, factory
        )
        if blueprint is None or not unreal.EditorAssetLibrary.save_loaded_asset(blueprint):
            raise RuntimeError(f"Could not create/save {anim_path}")
        unreal.log(f"Created animation Blueprint shell: {anim_path}")
    else:
        unreal.log(f"Preserved existing asset: {anim_path}")

    unreal.log("Phase 0-2 Blueprint shells ready for MCP authoring; gameplay is not yet wired.")


main()
