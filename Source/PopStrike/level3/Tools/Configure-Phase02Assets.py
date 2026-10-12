"""Author phase 0-2 defaults in the open editor; invoked through MCP, no PIE."""
import unreal


def asset(path):
    obj = unreal.load_asset(path)
    if obj is None:
        raise RuntimeError(f"Missing required asset: {path}")
    return obj


def data(name, cls):
    path = f"/Game/PopStrike/Input/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return asset(path), False
    result = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, "/Game/PopStrike/Input", cls, unreal.DataAssetFactory())
    if result is None:
        raise RuntimeError(f"Could not create {path}")
    return result, True


def save(obj):
    if not unreal.EditorAssetLibrary.save_loaded_asset(obj):
        raise RuntimeError(f"Could not save {obj.get_path_name()}")


def run():
    actions = {}
    for name in ("Move", "Look", "Jump", "Crouch", "Fire", "Reload"):
        action, created = data(f"IA_{name}", unreal.InputAction)
        if created:
            action.set_editor_property("value_type", unreal.InputActionValueType.AXIS2D
                if name in ("Move", "Look") else unreal.InputActionValueType.BOOLEAN)
            save(action)
        actions[name] = action

    context, created = data("IMC_Match", unreal.InputMappingContext)
    if created:
        mappings = []

        def mapping(action, key, negate=False, swizzle=False):
            modifiers = []
            if negate:
                modifiers.append(unreal.InputModifierNegate(outer=context))
            if swizzle:
                modifier = unreal.InputModifierSwizzleAxis(outer=context)
                modifier.set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
                modifiers.append(modifier)
            item = unreal.EnhancedActionKeyMapping()
            item.set_editor_property("action", actions[action])
            item.set_editor_property("key", unreal.Key(key))
            item.set_editor_property("modifiers", modifiers)
            mappings.append(item)

        mapping("Move", "W", swizzle=True)
        mapping("Move", "S", negate=True, swizzle=True)
        mapping("Move", "D")
        mapping("Move", "A", negate=True)
        mapping("Look", "MouseX")
        mapping("Look", "MouseY", negate=True, swizzle=True)
        mapping("Jump", "SpaceBar")
        mapping("Crouch", "LeftControl")
        mapping("Fire", "LeftMouseButton")
        mapping("Reload", "R")
        # UE 5.8 stores these in DefaultKeyMappings; the old Mappings property is deprecated.
        defaults = context.get_editor_property("default_key_mappings")
        defaults.set_editor_property("mappings", mappings)
        context.set_editor_property("default_key_mappings", defaults)
        save(context)

    folder = "/Game/PopStrike/Blueprints/"
    character = asset(folder + "BP_FPSCharacter")
    controller = asset(folder + "BP_MatchPlayerController")
    mode = asset(folder + "BP_MatchGameMode")
    weapon = asset(folder + "BP_Weapon_M4")
    animation = asset("/Game/PopStrike/Animations/ABP_FPSCharacter")
    definition = asset("/Game/PopStrike/Data/DA_M4")

    character_cdo = unreal.get_default_object(character.generated_class())
    mesh = character_cdo.get_editor_property("mesh")
    mesh.set_skeletal_mesh(asset("/Game/Characters/Heroes/Mannequin/Meshes/SKM_Manny"))
    mesh.set_anim_instance_class(animation.generated_class())
    # Imported post-process animation may depend on the original Lyra setup.
    mesh.set_editor_property("disable_post_process_blueprint", True)
    character_cdo.get_editor_property("combat").set_editor_property("starting_weapon", definition)

    controller_cdo = unreal.get_default_object(controller.generated_class())
    controller_cdo.set_editor_property("match_mapping", context)
    for name, action in actions.items():
        controller_cdo.set_editor_property(name.lower() + "_action", action)

    mode_cdo = unreal.get_default_object(mode.generated_class())
    mode_cdo.set_editor_property("default_pawn_class", character.generated_class())
    mode_cdo.set_editor_property("player_controller_class", controller.generated_class())
    definition.set_editor_property("weapon_class", weapon.generated_class())
    definition.set_editor_property("reload_duration",
        asset("/Game/Weapons/Rifle/Animations/Weap_Rifle_Reload").get_editor_property("sequence_length"))
    # Starting view-model placement; final hand alignment requires the user's visual play check.
    definition.set_editor_property("first_person_transform", unreal.Transform(
        location=unreal.Vector(35, 14, -18), rotation=unreal.Rotator(0, 0, 0),
        scale=unreal.Vector(1, 1, 1)))
    for blueprint in (character, controller, mode, weapon):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        save(blueprint)
    save(animation)
    save(definition)
    unreal.log("PHASE02_DEFAULTS_AUTHORED: input, character animation, combat and GameMode defaults saved; no PIE run.")


run()
