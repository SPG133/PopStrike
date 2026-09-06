# -*- coding: utf-8 -*-
"""Build the PopStrike main-menu and lobby Widget Blueprints."""

import unreal


ASSET_ROOT = "/Game/UI/Lobby"


def color(value, alpha=1.0):
    value = value.lstrip("#")
    return unreal.LinearColor(
        int(value[0:2], 16) / 255.0,
        int(value[2:4], 16) / 255.0,
        int(value[4:6], 16) / 255.0,
        alpha,
    )


PALETTE = {
    "bg": color("171526"),
    "panel": color("221F35", 0.94),
    "text": color("F2F2F2"),
    "dim": color("9A93B8"),
    "error": color("FF6B6B"),
    "red": color("FF4655"),
    "orange": color("FF9F1C"),
    "cyan": color("1B9AAA"),
    "violet": color("7B2CBF"),
    "gold": color("FFD23F"),
    "cta": color("FF3860"),
    "cta_pressed": color("C71F45"),
    "leave": color("39374F"),
}


COPY = {
    "title": "爆 破 突 击",
    "subtitle": "三对三，炸个痛快",
    "name_label": "你的代号",
    "name_hint": "喊个响亮的代号吧",
    "server_label": "目标服务器",
    "server_hint": "IP:端口，例如 127.0.0.1:7777",
    "error": "地址不能为空，兄弟",
    "menu_start": "开    战",
    "version": "v0.1  内部测试版",
    "room_title": "备 战 大 厅",
    "room_hint": "等待房主拉响警报……",
    "team_a": "烈焰突击组",
    "team_b": "冰锋防线组",
    "empty": "虚位以待",
    "host": "房主",
    "lobby_start": "拉响警报",
    "leave": "撤    退",
}


def try_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
        return True
    except Exception as exc:
        unreal.log_warning("[PopStrikeUI] %s.%s: %s" % (obj.get_name(), prop, exc))
        return False


def enum_value(enum_type, name):
    try:
        return getattr(enum_type, name)
    except Exception:
        return None


def make_widget_blueprint(name):
    path = "%s/%s" % (ASSET_ROOT, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.load_asset(path)
        if not existing:
            raise RuntimeError("Asset exists but could not be loaded: %s" % path)
        unreal.log_warning("[PopStrikeUI] Reusing incomplete asset from a previous run: %s" % path)
        return existing
    factory = unreal.WidgetBlueprintFactory()
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, ASSET_ROOT, unreal.WidgetBlueprint, factory
    )
    if not asset:
        raise RuntimeError("Unable to create %s" % path)
    return asset


def widget_tree(blueprint):
    try:
        return blueprint.widget_tree
    except Exception:
        pass
    try:
        return blueprint.get_editor_property("WidgetTree")
    except Exception:
        pass

    # Newer engine branches protect UWidgetBlueprint::WidgetTree from reflected
    # property access. It is still an instanced subobject owned by the asset, so
    # resolve that subobject directly by object path instead.
    tree = unreal.find_object(None, blueprint.get_path_name() + ":WidgetTree")
    if tree:
        return tree
    tree = unreal.find_object(None, blueprint.get_path_name() + ".WidgetTree")
    if tree:
        return tree
    raise RuntimeError("Could not resolve WidgetTree subobject for %s" % blueprint.get_path_name())


def generated_class(blueprint):
    candidate = getattr(blueprint, "generated_class", None)
    if callable(candidate):
        return candidate()
    if candidate:
        return candidate
    return blueprint.get_editor_property("GeneratedClass")


def construct(tree, widget_type, name):
    return tree.construct_widget(widget_type, name)


def set_root(tree, widget):
    try:
        tree.root_widget = widget
    except Exception:
        tree.set_editor_property("RootWidget", widget)


def anchors(minimum, maximum):
    return unreal.Anchors(unreal.Vector2D(*minimum), unreal.Vector2D(*maximum))

# -*- coding: utf-8 -*-
"""Build the PopStrike main-menu and lobby Widget Blueprints for UE 5.8.

Key change vs the original script:
- Do NOT call WidgetTree.construct_widget() (not exposed to Python in UE 5.8).
- Use EditorUtilityLibrary.add_source_widget() to create + parent widgets.
- Configure the slot that Unreal creates automatically.

Existing generated assets are backed up (renamed) before a rebuild, never deleted.
"""

from datetime import datetime
import unreal


ASSET_ROOT = "/Game/UI/Lobby"
BACKUP_ROOT = ASSET_ROOT + "/_Backup"
GENERATED_ASSETS = ("WBP_Lobby", "WBP_MainMenu", "WBP_PlayerSlot")


def color(value, alpha=1.0):
    value = value.lstrip("#")
    return unreal.LinearColor(
        int(value[0:2], 16) / 255.0,
        int(value[2:4], 16) / 255.0,
        int(value[4:6], 16) / 255.0,
        alpha,
    )


PALETTE = {
    "bg": color("171526"),
    "panel": color("221F35", 0.94),
    "text": color("F2F2F2"),
    "dim": color("9A93B8"),
    "error": color("FF6B6B"),
    "red": color("FF4655"),
    "orange": color("FF9F1C"),
    "cyan": color("1B9AAA"),
    "violet": color("7B2CBF"),
    "gold": color("FFD23F"),
    "cta": color("FF3860"),
    "cta_pressed": color("C71F45"),
    "leave": color("39374F"),
}


COPY = {
    "title": "爆 破 突 击",
    "subtitle": "三对三，炸个痛快",
    "name_label": "你的代号",
    "name_hint": "喊个响亮的代号吧",
    "server_label": "目标服务器",
    "server_hint": "IP:端口，例如 127.0.0.1:7777",
    "error": "地址不能为空，兄弟",
    "menu_start": "开    战",
    "version": "v0.1  内部测试版",
    "room_title": "备 战 大 厅",
    "room_hint": "等待房主拉响警报……",
    "team_a": "烈焰突击组",
    "team_b": "冰锋防线组",
    "empty": "虚位以待",
    "host": "房主",
    "lobby_start": "拉响警报",
    "leave": "撤    退",
}


def log(message):
    unreal.log("[PopStrikeUI] %s" % message)


def warn(message):
    unreal.log_warning("[PopStrikeUI] %s" % message)


def slate_color(value):
    try:
        return unreal.SlateColor(specified_color=value)
    except Exception:
        return unreal.SlateColor(value)


def try_get(obj, *props):
    last_exc = None
    for prop in props:
        try:
            return obj.get_editor_property(prop)
        except Exception as exc:
            last_exc = exc
    if last_exc:
        raise last_exc
    raise RuntimeError("No property names supplied")


def try_set(obj, prop, value, quiet=False):
    """Set a Python editor property; accept either one name or fallback names."""
    names = prop if isinstance(prop, (tuple, list)) else (prop,)
    last_exc = None
    for name in names:
        try:
            obj.set_editor_property(name, value)
            return True
        except Exception as exc:
            last_exc = exc
    if not quiet:
        warn("%s.%s: %s" % (obj.get_name(), "/".join(names), last_exc))
    return False


def set_angle(widget, angle):
    try:
        widget.set_render_transform_angle(float(angle))
        return True
    except Exception as exc:
        warn("%s.set_render_transform_angle: %s" % (widget.get_name(), exc))
        return False


def enum_value(enum_type, name):
    """Support both current H_ALIGN/V_ALIGN enum names and old spelling."""
    candidates = [name]
    if name.startswith("HALIGN_"):
        candidates.append(name.replace("HALIGN_", "H_ALIGN_"))
    if name.startswith("VALIGN_"):
        candidates.append(name.replace("VALIGN_", "V_ALIGN_"))
    if name.startswith("H_ALIGN_"):
        candidates.append(name.replace("H_ALIGN_", "HALIGN_"))
    if name.startswith("V_ALIGN_"):
        candidates.append(name.replace("V_ALIGN_", "VALIGN_"))

    for candidate in candidates:
        try:
            return getattr(enum_type, candidate)
        except Exception:
            pass
    return None


def backup_existing_assets():
    """Preserve prior/partial generated assets by renaming them into _Backup."""
    unreal.EditorAssetLibrary.make_directory(ASSET_ROOT)

    existing = []
    for name in GENERATED_ASSETS:
        path = "%s/%s" % (ASSET_ROOT, name)
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            existing.append((name, path))

    if not existing:
        return

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_dir = "%s/%s" % (BACKUP_ROOT, timestamp)
    unreal.EditorAssetLibrary.make_directory(backup_dir)

    # Rename dependents first, then WBP_PlayerSlot, to minimize reference churn.
    for name, source_path in existing:
        destination = "%s/%s" % (backup_dir, name)
        if not unreal.EditorAssetLibrary.rename_asset(source_path, destination):
            raise RuntimeError(
                "Could not back up existing asset: %s -> %s" %
                (source_path, destination)
            )
        log("Backed up existing asset: %s -> %s" % (source_path, destination))


def make_widget_blueprint(name):
    path = "%s/%s" % (ASSET_ROOT, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        raise RuntimeError(
            "Output asset already exists after backup step: %s" % path
        )

    factory = unreal.WidgetBlueprintFactory()
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name,
        ASSET_ROOT,
        unreal.WidgetBlueprint,
        factory,
    )
    if not asset:
        raise RuntimeError("Unable to create %s" % path)
    return asset


def add_widget(blueprint, widget_class, name, parent=None):
    """UE 5.8-safe UMG creation path: create and parent in one call."""
    parent_name = unreal.Name(parent) if parent else unreal.Name("None")
    widget = unreal.EditorUtilityLibrary.add_source_widget(
        blueprint,
        widget_class,
        unreal.Name(name),
        parent_name,
    )
    if not widget:
        raise RuntimeError(
            "Failed to add widget %s (%s), parent=%s" %
            (name, widget_class, parent or "<ROOT>")
        )
    return widget


def get_slot(widget):
    try:
        return try_get(widget, "slot", "Slot")
    except Exception as exc:
        warn("Could not read slot for %s: %s" % (widget.get_name(), exc))
        return None


def generated_class(blueprint):
    candidate = getattr(blueprint, "generated_class", None)
    if callable(candidate):
        result = candidate()
        if result:
            return result
    elif candidate:
        return candidate

    try:
        return try_get(blueprint, "generated_class", "GeneratedClass")
    except Exception:
        pass

    asset_path = blueprint.get_path_name().split(".")[0]
    try:
        result = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
        if result:
            return result
    except Exception:
        pass

    raise RuntimeError("Could not resolve generated class for %s" % blueprint.get_path_name())


def anchors(minimum, maximum):
    return unreal.Anchors(
        unreal.Vector2D(*minimum),
        unreal.Vector2D(*maximum),
    )


def canvas_fill(child, margin=0.0):
    slot = get_slot(child)
    if not slot:
        return None
    try:
        slot.set_anchors(anchors((0.0, 0.0), (1.0, 1.0)))
        slot.set_offsets(unreal.Margin(margin, margin, margin, margin))
    except Exception as exc:
        warn("Canvas fill failed for %s: %s" % (child.get_name(), exc))
    return slot


def canvas_point(child, anchor, alignment, position, size):
    slot = get_slot(child)
    if not slot:
        return None
    try:
        slot.set_anchors(anchors(anchor, anchor))
        slot.set_alignment(unreal.Vector2D(*alignment))
        slot.set_offsets(
            unreal.Margin(position[0], position[1], size[0], size[1])
        )
    except Exception as exc:
        warn("Canvas point failed for %s: %s" % (child.get_name(), exc))
    return slot


def fill_size():
    try:
        return unreal.SlateChildSize(
            size_rule=unreal.SlateSizeRule.FILL,
            value=1.0,
        )
    except Exception:
        return unreal.SlateChildSize(
            unreal.SlateSizeRule.FILL,
            1.0,
        )


def configure_box_slot(
    child,
    fill=False,
    padding=(0, 0, 0, 0),
    h="H_ALIGN_FILL",
    v="V_ALIGN_FILL",
):
    slot = get_slot(child)
    if not slot:
        return None

    try_set(slot, ("padding", "Padding"), unreal.Margin(*padding))

    h_value = enum_value(unreal.HorizontalAlignment, h)
    v_value = enum_value(unreal.VerticalAlignment, v)

    if h_value is not None:
        try:
            slot.set_horizontal_alignment(h_value)
        except Exception:
            try_set(slot, ("horizontal_alignment", "HorizontalAlignment"), h_value)

    if v_value is not None:
        try:
            slot.set_vertical_alignment(v_value)
        except Exception:
            try_set(slot, ("vertical_alignment", "VerticalAlignment"), v_value)

    if fill:
        try_set(slot, ("size", "Size"), fill_size())

    return slot


def add_size_box(blueprint, name, parent, width=None, height=None):
    box = add_widget(blueprint, unreal.SizeBox, name, parent)

    if width is not None:
        try:
            box.set_width_override(float(width))
        except Exception:
            try_set(box, ("width_override", "WidthOverride"), float(width))
            try_set(
                box,
                ("override_width_override", "bOverride_WidthOverride"),
                True,
                quiet=True,
            )

    if height is not None:
        try:
            box.set_height_override(float(height))
        except Exception:
            try_set(box, ("height_override", "HeightOverride"), float(height))
            try_set(
                box,
                ("override_height_override", "bOverride_HeightOverride"),
                True,
                quiet=True,
            )

    return box


def set_border_color(border, value):
    try_set(border, ("brush_color", "BrushColor"), value)


def set_text(widget, value, size, tint, bold=True, outline=0):
    widget.set_text(unreal.Text(value))

    try:
        font = try_get(widget, "font", "Font")
        try_set(font, ("size", "Size"), float(size))
        try_set(
            font,
            ("typeface_font_name", "TypefaceFontName"),
            unreal.Name("Bold" if bold else "Regular"),
            quiet=True,
        )

        if outline:
            try:
                settings = try_get(font, "outline_settings", "OutlineSettings")
                try_set(settings, ("outline_size", "OutlineSize"), int(outline))
                try_set(settings, ("outline_color", "OutlineColor"), PALETTE["bg"])
                try_set(font, ("outline_settings", "OutlineSettings"), settings)
            except Exception as exc:
                warn("Font outline on %s: %s" % (widget.get_name(), exc))

        try:
            widget.set_font(font)
        except Exception:
            try_set(widget, ("font", "Font"), font)
    except Exception as exc:
        warn("Font setup on %s: %s" % (widget.get_name(), exc))

    try:
        widget.set_color_and_opacity(slate_color(tint))
    except Exception:
        try_set(
            widget,
            ("color_and_opacity", "ColorAndOpacity"),
            slate_color(tint),
        )


def style_button(button, normal, pressed=None):
    try:
        style = try_get(button, "widget_style", "WidgetStyle")
        for state_name in ("normal", "hovered", "pressed"):
            try:
                brush = try_get(style, state_name, state_name.capitalize())
                tint = pressed if state_name == "pressed" and pressed else normal
                try_set(brush, ("tint_color", "TintColor"), slate_color(tint))
                try_set(style, (state_name, state_name.capitalize()), brush)
            except Exception as exc:
                warn("Button style %s.%s: %s" % (button.get_name(), state_name, exc))
        try_set(button, ("widget_style", "WidgetStyle"), style)
    except Exception as exc:
        warn("Button style on %s: %s" % (button.get_name(), exc))
        # Fallback still gives the button the requested color.
        try_set(button, ("background_color", "BackgroundColor"), normal, quiet=True)


def compile_save(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint):
        raise RuntimeError("Failed to save %s" % blueprint.get_path_name())
    return blueprint


def build_player_slot():
    blueprint = make_widget_blueprint("WBP_PlayerSlot")

    add_widget(blueprint, unreal.Overlay, "Root")

    background = add_widget(blueprint, unreal.Border, "Bg_Slot", "Root")
    set_border_color(background, PALETTE["panel"])
    try_set(background, ("padding", "Padding"), unreal.Margin(14, 10, 14, 10))

    add_widget(blueprint, unreal.HorizontalBox, "Content", "Bg_Slot")

    avatar_box = add_size_box(
        blueprint,
        "Box_Avatar",
        "Content",
        width=44,
        height=44,
    )
    configure_box_slot(
        avatar_box,
        padding=(0, 0, 12, 0),
        h="H_ALIGN_LEFT",
        v="V_ALIGN_CENTER",
    )

    avatar = add_widget(
        blueprint,
        unreal.Border,
        "Img_AvatarPlaceholder",
        "Box_Avatar",
    )
    set_border_color(avatar, PALETTE["dim"])
    set_angle(avatar, 6.0)

    info = add_widget(blueprint, unreal.VerticalBox, "InfoBox", "Content")
    configure_box_slot(info, fill=True, v="V_ALIGN_CENTER")

    player_name = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_PlayerName",
        "InfoBox",
    )
    set_text(player_name, COPY["empty"], 18, PALETTE["dim"])
    configure_box_slot(player_name)

    badge = add_size_box(
        blueprint,
        "Badge_Host",
        "Root",
        width=48,
        height=22,
    )
    try:
        badge.set_visibility(unreal.SlateVisibility.COLLAPSED)
    except Exception:
        try_set(badge, ("visibility", "Visibility"), unreal.SlateVisibility.COLLAPSED)

    badge_slot = get_slot(badge)
    if badge_slot:
        h = enum_value(unreal.HorizontalAlignment, "H_ALIGN_RIGHT")
        v = enum_value(unreal.VerticalAlignment, "V_ALIGN_TOP")
        if h is not None:
            try:
                badge_slot.set_horizontal_alignment(h)
            except Exception:
                try_set(badge_slot, ("horizontal_alignment", "HorizontalAlignment"), h)
        if v is not None:
            try:
                badge_slot.set_vertical_alignment(v)
            except Exception:
                try_set(badge_slot, ("vertical_alignment", "VerticalAlignment"), v)

    badge_bg = add_widget(
        blueprint,
        unreal.Border,
        "Bg_HostBadge",
        "Badge_Host",
    )
    set_border_color(badge_bg, PALETTE["gold"])

    badge_text = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_HostBadge",
        "Bg_HostBadge",
    )
    set_text(badge_text, COPY["host"], 12, PALETTE["bg"])

    return compile_save(blueprint)


def build_main_menu():
    blueprint = make_widget_blueprint("WBP_MainMenu")

    add_widget(blueprint, unreal.CanvasPanel, "RootCanvas")

    background = add_widget(blueprint, unreal.Border, "Bg_Base", "RootCanvas")
    set_border_color(background, PALETTE["bg"])
    canvas_fill(background)

    stripe_a = add_widget(
        blueprint,
        unreal.Border,
        "Bg_AccentStripeA",
        "RootCanvas",
    )
    set_border_color(stripe_a, PALETTE["red"])
    canvas_point(stripe_a, (0, 1), (0, 1), (-160, 160), (700, 340))
    set_angle(stripe_a, -8.0)

    stripe_b = add_widget(
        blueprint,
        unreal.Border,
        "Bg_AccentStripeB",
        "RootCanvas",
    )
    set_border_color(stripe_b, PALETTE["cyan"])
    canvas_point(stripe_b, (1, 0), (1, 0), (160, -160), (700, 340))
    set_angle(stripe_b, -8.0)

    center = add_widget(
        blueprint,
        unreal.VerticalBox,
        "CenterStack",
        "RootCanvas",
    )
    canvas_point(center, (0.5, 0.5), (0.5, 0.5), (0, 0), (520, 520))

    title = add_widget(blueprint, unreal.TextBlock, "Txt_Title", "CenterStack")
    set_text(title, COPY["title"], 60, PALETTE["text"], outline=3)
    set_angle(title, -3.0)
    configure_box_slot(
        title,
        padding=(0, 0, 0, 6),
        h="H_ALIGN_CENTER",
    )

    subtitle = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_Subtitle",
        "CenterStack",
    )
    set_text(subtitle, COPY["subtitle"], 18, PALETTE["dim"], bold=False)
    configure_box_slot(
        subtitle,
        padding=(0, 0, 0, 32),
        h="H_ALIGN_CENTER",
    )

    name_label = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_NameLabel",
        "CenterStack",
    )
    set_text(name_label, COPY["name_label"], 15, PALETTE["dim"])
    configure_box_slot(name_label, padding=(4, 0, 0, 4))

    name_input = add_widget(
        blueprint,
        unreal.EditableTextBox,
        "Edt_PlayerName",
        "CenterStack",
    )
    try_set(name_input, ("hint_text", "HintText"), unreal.Text(COPY["name_hint"]))
    configure_box_slot(name_input, padding=(0, 0, 0, 20))

    server_label = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_ServerLabel",
        "CenterStack",
    )
    set_text(server_label, COPY["server_label"], 15, PALETTE["dim"])
    configure_box_slot(server_label, padding=(4, 0, 0, 4))

    server_input = add_widget(
        blueprint,
        unreal.EditableTextBox,
        "Edt_ServerAddress",
        "CenterStack",
    )
    try_set(
        server_input,
        ("hint_text", "HintText"),
        unreal.Text(COPY["server_hint"]),
    )
    configure_box_slot(server_input, padding=(0, 0, 0, 12))

    error = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_ErrorMessage",
        "CenterStack",
    )
    set_text(error, COPY["error"], 13, PALETTE["error"], bold=False)
    try:
        error.set_visibility(unreal.SlateVisibility.COLLAPSED)
    except Exception:
        try_set(error, ("visibility", "Visibility"), unreal.SlateVisibility.COLLAPSED)
    configure_box_slot(error, padding=(0, 0, 0, 20))

    start = add_widget(
        blueprint,
        unreal.Button,
        "Btn_StartGame",
        "CenterStack",
    )
    style_button(start, PALETTE["cta"], PALETTE["cta_pressed"])
    set_angle(start, -2.0)
    configure_box_slot(start, h="H_ALIGN_CENTER")

    start_text = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_StartGameLabel",
        "Btn_StartGame",
    )
    set_text(start_text, COPY["menu_start"], 20, PALETTE["text"], outline=1)

    version = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_VersionTag",
        "RootCanvas",
    )
    set_text(version, COPY["version"], 11, PALETTE["dim"], bold=False)
    canvas_point(version, (1, 1), (1, 1), (-16, -12), (220, 20))

    return compile_save(blueprint)


def build_lobby(slot_blueprint):
    blueprint = make_widget_blueprint("WBP_Lobby")

    add_widget(blueprint, unreal.CanvasPanel, "RootCanvas")

    background = add_widget(blueprint, unreal.Border, "Bg_Base", "RootCanvas")
    set_border_color(background, PALETTE["bg"])
    canvas_fill(background)

    stripe_a = add_widget(
        blueprint,
        unreal.Border,
        "Bg_AccentStripeA",
        "RootCanvas",
    )
    set_border_color(stripe_a, PALETTE["red"])
    canvas_point(stripe_a, (0, 0), (0, 0), (-220, -200), (600, 300))
    set_angle(stripe_a, -8.0)

    stripe_b = add_widget(
        blueprint,
        unreal.Border,
        "Bg_AccentStripeB",
        "RootCanvas",
    )
    set_border_color(stripe_b, PALETTE["cyan"])
    canvas_point(stripe_b, (1, 1), (1, 1), (220, 200), (600, 300))
    set_angle(stripe_b, -8.0)

    main = add_widget(
        blueprint,
        unreal.VerticalBox,
        "MainStack",
        "RootCanvas",
    )
    canvas_fill(main, 48)

    top = add_widget(blueprint, unreal.HorizontalBox, "TopBar", "MainStack")
    configure_box_slot(top, padding=(0, 0, 0, 24))

    room_title = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_RoomTitle",
        "TopBar",
    )
    set_text(room_title, COPY["room_title"], 32, PALETTE["text"], outline=2)
    configure_box_slot(
        room_title,
        fill=True,
        h="H_ALIGN_LEFT",
        v="V_ALIGN_CENTER",
    )

    room_hint = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_RoomStatusHint",
        "TopBar",
    )
    set_text(room_hint, COPY["room_hint"], 15, PALETTE["dim"], bold=False)
    configure_box_slot(
        room_hint,
        h="H_ALIGN_RIGHT",
        v="V_ALIGN_CENTER",
    )

    teams = add_widget(
        blueprint,
        unreal.HorizontalBox,
        "TeamsRow",
        "MainStack",
    )
    configure_box_slot(teams, fill=True, padding=(0, 0, 0, 24))

    slot_class = generated_class(slot_blueprint)

    def add_team_panel(key, tint, label, padding):
        panel_name = "Panel_Team%s" % key
        panel = add_widget(
            blueprint,
            unreal.VerticalBox,
            panel_name,
            "TeamsRow",
        )
        configure_box_slot(panel, fill=True, padding=padding)

        header_name = "Header_Team%s" % key
        header = add_widget(
            blueprint,
            unreal.Border,
            header_name,
            panel_name,
        )
        set_border_color(header, tint)
        try_set(header, ("padding", "Padding"), unreal.Margin(12, 10, 12, 10))
        configure_box_slot(header, padding=(0, 0, 0, 12))

        header_text = add_widget(
            blueprint,
            unreal.TextBlock,
            "Txt_Team%sName" % key,
            header_name,
        )
        set_text(header_text, label, 20, PALETTE["text"], outline=1)
        try_set(
            header_text,
            ("justification", "Justification"),
            unreal.TextJustify.CENTER,
        )

        slot_list_name = "SlotList_Team%s" % key
        slot_list = add_widget(
            blueprint,
            unreal.VerticalBox,
            slot_list_name,
            panel_name,
        )
        configure_box_slot(slot_list, fill=True)

        for index in range(1, 4):
            player_slot = add_widget(
                blueprint,
                slot_class,
                "Slot_%s%d" % (key, index),
                slot_list_name,
            )
            configure_box_slot(player_slot, padding=(0, 0, 0, 8))

        return panel

    add_team_panel("A", PALETTE["red"], COPY["team_a"], (0, 12, 0, 0))
    add_team_panel("B", PALETTE["cyan"], COPY["team_b"], (12, 0, 0, 0))

    bottom = add_widget(
        blueprint,
        unreal.HorizontalBox,
        "BottomBar",
        "MainStack",
    )
    configure_box_slot(bottom)

    leave = add_widget(
        blueprint,
        unreal.Button,
        "Btn_LeaveRoom",
        "BottomBar",
    )
    style_button(leave, PALETTE["leave"])
    configure_box_slot(leave, h="H_ALIGN_LEFT")

    leave_text = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_LeaveLabel",
        "Btn_LeaveRoom",
    )
    set_text(leave_text, COPY["leave"], 17, PALETTE["text"])

    start = add_widget(
        blueprint,
        unreal.Button,
        "Btn_StartGame",
        "BottomBar",
    )
    style_button(start, PALETTE["cta"], PALETTE["cta_pressed"])
    set_angle(start, -2.0)
    configure_box_slot(start, fill=True, h="H_ALIGN_RIGHT")

    start_text = add_widget(
        blueprint,
        unreal.TextBlock,
        "Txt_StartGameLabel",
        "Btn_StartGame",
    )
    set_text(start_text, COPY["lobby_start"], 17, PALETTE["text"], outline=1)

    return compile_save(blueprint)


def main():
    log("UE 5.8 add_source_widget build starting")
    backup_existing_assets()

    slot = build_player_slot()
    main_menu = build_main_menu()
    lobby = build_lobby(slot)

    log("Created WBP_MainMenu, WBP_Lobby and WBP_PlayerSlot")
    log("Output: %s" % ASSET_ROOT)
    return main_menu, lobby, slot


if __name__ == "__main__":
    main()

def canvas_fill(canvas, child, margin=0.0):
    slot = canvas.add_child(child)
    slot.set_anchors(anchors((0.0, 0.0), (1.0, 1.0)))
    slot.set_offsets(unreal.Margin(margin, margin, margin, margin))
    return slot


def canvas_point(canvas, child, anchor, alignment, position, size):
    slot = canvas.add_child(child)
    slot.set_anchors(anchors(anchor, anchor))
    slot.set_alignment(unreal.Vector2D(*alignment))
    slot.set_offsets(unreal.Margin(position[0], position[1], size[0], size[1]))
    return slot


def fill_size():
    try:
        return unreal.SlateChildSize(unreal.SlateSizeRule.FILL, 1.0)
    except Exception:
        return unreal.SlateChildSize(size_rule=unreal.SlateSizeRule.FILL, value=1.0)


def box_add(box, child, fill=False, padding=(0, 0, 0, 0), h="HALIGN_FILL", v="VALIGN_FILL"):
    slot = box.add_child(child)
    try_set(slot, "Padding", unreal.Margin(*padding))
    h_value = enum_value(unreal.HorizontalAlignment, h)
    v_value = enum_value(unreal.VerticalAlignment, v)
    if h_value is not None:
        try_set(slot, "HorizontalAlignment", h_value)
    if v_value is not None:
        try_set(slot, "VerticalAlignment", v_value)
    if fill:
        try_set(slot, "Size", fill_size())
    return slot


def fixed_size(tree, name, child, width, height):
    box = construct(tree, unreal.SizeBox, name)
    try_set(box, "WidthOverride", float(width))
    try_set(box, "bOverride_WidthOverride", True)
    try_set(box, "HeightOverride", float(height))
    try_set(box, "bOverride_HeightOverride", True)
    box.add_child(child)
    return box


def set_border_color(border, value):
    try_set(border, "BrushColor", value)


def set_text(widget, value, size, tint, bold=True, outline=0):
    widget.set_text(unreal.Text(value))
    font = widget.get_editor_property("Font")
    try_set(font, "Size", size)
    try_set(font, "TypefaceFontName", "Bold" if bold else "Regular")
    if outline:
        settings = font.get_editor_property("OutlineSettings")
        try_set(settings, "OutlineSize", outline)
        try_set(settings, "OutlineColor", PALETTE["bg"])
        try_set(font, "OutlineSettings", settings)
    try_set(widget, "Font", font)
    try_set(widget, "ColorAndOpacity", unreal.SlateColor(tint))


def style_button(button, normal, pressed=None):
    style = button.get_editor_property("WidgetStyle")
    for state_name in ("Normal", "Hovered", "Pressed"):
        brush = style.get_editor_property(state_name)
        tint = pressed if state_name == "Pressed" and pressed else normal
        try_set(brush, "TintColor", unreal.SlateColor(tint))
        style.set_editor_property(state_name, brush)
    try_set(button, "WidgetStyle", style)


def compile_save(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint):
        raise RuntimeError("Failed to save %s" % blueprint.get_path_name())
    return blueprint


def build_player_slot():
    blueprint = make_widget_blueprint("WBP_PlayerSlot")
    tree = widget_tree(blueprint)
    root = construct(tree, unreal.Overlay, "Root")
    set_root(tree, root)

    background = construct(tree, unreal.Border, "Bg_Slot")
    set_border_color(background, PALETTE["panel"])
    try_set(background, "Padding", unreal.Margin(14, 10, 14, 10))
    root.add_child(background)

    content = construct(tree, unreal.HorizontalBox, "Content")
    background.add_child(content)

    avatar = construct(tree, unreal.Border, "Img_AvatarPlaceholder")
    set_border_color(avatar, PALETTE["dim"])
    try_set(avatar, "RenderTransformAngle", 6.0)
    box_add(content, fixed_size(tree, "Box_Avatar", avatar, 44, 44), padding=(0, 0, 12, 0), h="HALIGN_LEFT", v="VALIGN_CENTER")

    info = construct(tree, unreal.VerticalBox, "InfoBox")
    box_add(content, info, fill=True, v="VALIGN_CENTER")
    player_name = construct(tree, unreal.TextBlock, "Txt_PlayerName")
    set_text(player_name, COPY["empty"], 18, PALETTE["dim"])
    box_add(info, player_name)

    badge_bg = construct(tree, unreal.Border, "Bg_HostBadge")
    set_border_color(badge_bg, PALETTE["gold"])
    badge_text = construct(tree, unreal.TextBlock, "Txt_HostBadge")
    set_text(badge_text, COPY["host"], 12, PALETTE["bg"])
    badge_bg.add_child(badge_text)
    badge = fixed_size(tree, "Badge_Host", badge_bg, 48, 22)
    try_set(badge, "Visibility", unreal.SlateVisibility.COLLAPSED)
    badge_slot = root.add_child(badge)
    try_set(badge_slot, "HorizontalAlignment", unreal.HorizontalAlignment.HALIGN_RIGHT)
    try_set(badge_slot, "VerticalAlignment", unreal.VerticalAlignment.VALIGN_TOP)
    return compile_save(blueprint)


def build_main_menu():
    blueprint = make_widget_blueprint("WBP_MainMenu")
    tree = widget_tree(blueprint)
    root = construct(tree, unreal.CanvasPanel, "RootCanvas")
    set_root(tree, root)

    background = construct(tree, unreal.Border, "Bg_Base")
    set_border_color(background, PALETTE["bg"])
    canvas_fill(root, background)

    stripe_a = construct(tree, unreal.Border, "Bg_AccentStripeA")
    set_border_color(stripe_a, PALETTE["red"])
    canvas_point(root, stripe_a, (0, 1), (0, 1), (-160, 160), (700, 340))
    try_set(stripe_a, "RenderTransformAngle", -8.0)

    stripe_b = construct(tree, unreal.Border, "Bg_AccentStripeB")
    set_border_color(stripe_b, PALETTE["cyan"])
    canvas_point(root, stripe_b, (1, 0), (1, 0), (160, -160), (700, 340))
    try_set(stripe_b, "RenderTransformAngle", -8.0)

    center = construct(tree, unreal.VerticalBox, "CenterStack")
    canvas_point(root, center, (0.5, 0.5), (0.5, 0.5), (0, 0), (520, 520))

    title = construct(tree, unreal.TextBlock, "Txt_Title")
    set_text(title, COPY["title"], 60, PALETTE["text"], outline=3)
    try_set(title, "RenderTransformAngle", -3.0)
    box_add(center, title, padding=(0, 0, 0, 6), h="HALIGN_CENTER")

    subtitle = construct(tree, unreal.TextBlock, "Txt_Subtitle")
    set_text(subtitle, COPY["subtitle"], 18, PALETTE["dim"], bold=False)
    box_add(center, subtitle, padding=(0, 0, 0, 32), h="HALIGN_CENTER")

    name_label = construct(tree, unreal.TextBlock, "Txt_NameLabel")
    set_text(name_label, COPY["name_label"], 15, PALETTE["dim"])
    box_add(center, name_label, padding=(4, 0, 0, 4))
    name_input = construct(tree, unreal.EditableTextBox, "Edt_PlayerName")
    try_set(name_input, "HintText", unreal.Text(COPY["name_hint"]))
    box_add(center, name_input, padding=(0, 0, 0, 20))

    server_label = construct(tree, unreal.TextBlock, "Txt_ServerLabel")
    set_text(server_label, COPY["server_label"], 15, PALETTE["dim"])
    box_add(center, server_label, padding=(4, 0, 0, 4))
    server_input = construct(tree, unreal.EditableTextBox, "Edt_ServerAddress")
    try_set(server_input, "HintText", unreal.Text(COPY["server_hint"]))
    box_add(center, server_input, padding=(0, 0, 0, 12))

    error = construct(tree, unreal.TextBlock, "Txt_ErrorMessage")
    set_text(error, COPY["error"], 13, PALETTE["error"], bold=False)
    try_set(error, "Visibility", unreal.SlateVisibility.COLLAPSED)
    box_add(center, error, padding=(0, 0, 0, 20))

    start = construct(tree, unreal.Button, "Btn_StartGame")
    style_button(start, PALETTE["cta"], PALETTE["cta_pressed"])
    start_text = construct(tree, unreal.TextBlock, "Txt_StartGameLabel")
    set_text(start_text, COPY["menu_start"], 20, PALETTE["text"], outline=1)
    start.add_child(start_text)
    try_set(start, "RenderTransformAngle", -2.0)
    box_add(center, start, h="HALIGN_CENTER")

    version = construct(tree, unreal.TextBlock, "Txt_VersionTag")
    set_text(version, COPY["version"], 11, PALETTE["dim"], bold=False)
    canvas_point(root, version, (1, 1), (1, 1), (-16, -12), (220, 20))
    return compile_save(blueprint)


def build_lobby(slot_blueprint):
    blueprint = make_widget_blueprint("WBP_Lobby")
    tree = widget_tree(blueprint)
    root = construct(tree, unreal.CanvasPanel, "RootCanvas")
    set_root(tree, root)

    background = construct(tree, unreal.Border, "Bg_Base")
    set_border_color(background, PALETTE["bg"])
    canvas_fill(root, background)

    stripe_a = construct(tree, unreal.Border, "Bg_AccentStripeA")
    set_border_color(stripe_a, PALETTE["red"])
    canvas_point(root, stripe_a, (0, 0), (0, 0), (-220, -200), (600, 300))
    try_set(stripe_a, "RenderTransformAngle", -8.0)
    stripe_b = construct(tree, unreal.Border, "Bg_AccentStripeB")
    set_border_color(stripe_b, PALETTE["cyan"])
    canvas_point(root, stripe_b, (1, 1), (1, 1), (220, 200), (600, 300))
    try_set(stripe_b, "RenderTransformAngle", -8.0)

    main = construct(tree, unreal.VerticalBox, "MainStack")
    canvas_fill(root, main, 48)
    top = construct(tree, unreal.HorizontalBox, "TopBar")
    box_add(main, top, padding=(0, 0, 0, 24))
    room_title = construct(tree, unreal.TextBlock, "Txt_RoomTitle")
    set_text(room_title, COPY["room_title"], 32, PALETTE["text"], outline=2)
    box_add(top, room_title, fill=True, h="HALIGN_LEFT", v="VALIGN_CENTER")
    room_hint = construct(tree, unreal.TextBlock, "Txt_RoomStatusHint")
    set_text(room_hint, COPY["room_hint"], 15, PALETTE["dim"], bold=False)
    box_add(top, room_hint, h="HALIGN_RIGHT", v="VALIGN_CENTER")

    teams = construct(tree, unreal.HorizontalBox, "TeamsRow")
    box_add(main, teams, fill=True, padding=(0, 0, 0, 24))
    slot_class = generated_class(slot_blueprint)

    def team_panel(key, tint, label):
        panel = construct(tree, unreal.VerticalBox, "Panel_Team%s" % key)
        header = construct(tree, unreal.Border, "Header_Team%s" % key)
        set_border_color(header, tint)
        try_set(header, "Padding", unreal.Margin(12, 10, 12, 10))
        header_text = construct(tree, unreal.TextBlock, "Txt_Team%sName" % key)
        set_text(header_text, label, 20, PALETTE["text"], outline=1)
        try_set(header_text, "Justification", unreal.TextJustify.CENTER)
        header.add_child(header_text)
        box_add(panel, header, padding=(0, 0, 0, 12))
        slots = construct(tree, unreal.VerticalBox, "SlotList_Team%s" % key)
        box_add(panel, slots, fill=True)
        for index in range(1, 4):
            player_slot = tree.construct_widget(slot_class, "Slot_%s%d" % (key, index))
            box_add(slots, player_slot, padding=(0, 0, 0, 8))
        return panel

    box_add(teams, team_panel("A", PALETTE["red"], COPY["team_a"]), fill=True, padding=(0, 12, 0, 0))
    box_add(teams, team_panel("B", PALETTE["cyan"], COPY["team_b"]), fill=True, padding=(12, 0, 0, 0))

    bottom = construct(tree, unreal.HorizontalBox, "BottomBar")
    box_add(main, bottom)
    leave = construct(tree, unreal.Button, "Btn_LeaveRoom")
    style_button(leave, PALETTE["leave"])
    leave_text = construct(tree, unreal.TextBlock, "Txt_LeaveLabel")
    set_text(leave_text, COPY["leave"], 17, PALETTE["text"])
    leave.add_child(leave_text)
    box_add(bottom, leave, h="HALIGN_LEFT")

    start = construct(tree, unreal.Button, "Btn_StartGame")
    style_button(start, PALETTE["cta"], PALETTE["cta_pressed"])
    start_text = construct(tree, unreal.TextBlock, "Txt_StartGameLabel")
    set_text(start_text, COPY["lobby_start"], 17, PALETTE["text"], outline=1)
    start.add_child(start_text)
    try_set(start, "RenderTransformAngle", -2.0)
    box_add(bottom, start, fill=True, h="HALIGN_RIGHT")
    return compile_save(blueprint)


def main():
    unreal.EditorAssetLibrary.make_directory(ASSET_ROOT)
    slot = build_player_slot()
    main_menu = build_main_menu()
    lobby = build_lobby(slot)
    unreal.log("[PopStrikeUI] Created WBP_MainMenu, WBP_Lobby and WBP_PlayerSlot")
    return main_menu, lobby, slot


if __name__ == "__main__":
    main()
