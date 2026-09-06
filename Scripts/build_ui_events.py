"""Run with UnrealEditor-Cmd -run=pythonscript -script=<absolute path>.
Backs up existing assets outside Content, preserves designers, creates native-backed event nodes.
"""
import unreal
import shutil
from pathlib import Path
from datetime import datetime

project = Path(unreal.Paths.project_dir()).resolve()
backup = project / 'Saved' / 'UIBackups' / datetime.now().strftime('%Y%m%d_%H%M%S')
backup.mkdir(parents=True)
specs = [
    ('WBP_PlayerSlot', '/Script/PopStrike.PSPlayerSlotWidget', []),
    ('WBP_MainMenu', '/Script/PopStrike.PSMainMenuWidget', [
        ('Btn_StartGame', 'OnStartClicked'), ('Btn_UploadAvatar', 'OnAvatarClicked')]),
    ('WBP_Lobby', '/Script/PopStrike.PSLobbyWidget', [
        ('Btn_JoinTeamA', 'JoinA'), ('Btn_JoinTeamB', 'JoinB'),
        ('Btn_MapPrev', 'PrevMap'), ('Btn_MapNext', 'NextMap'),
        ('Btn_StartGame', 'StartMatch'), ('Btn_LeaveRoom', 'Leave')]),
]
for name, _, _ in specs:
    shutil.copy2(project / 'Content/UI/Lobby' / (name + '.uasset'), backup / (name + '.uasset'))
reports = []
for name, parent, events in specs:
    bp = unreal.load_asset('/Game/UI/Lobby/' + name)
    assert bp, name
    reports.append('BEFORE ' + name + '\n' + unreal.PSUIBlueprintTools.inspect(bp))
    assert unreal.PSUIBlueprintTools.prepare(bp, unreal.load_class(None, parent)), 'Parent/binding compile: ' + name
    for row, (widget, function) in enumerate(events):
        assert unreal.PSUIBlueprintTools.wire_button(bp, widget, function, row), 'Event conflict or missing widget: ' + widget
    assert unreal.PSUIBlueprintTools.compile_checked(bp), 'Final compile: ' + name
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), 'Save: ' + name
    reports.append('AFTER ' + name + '\n' + unreal.PSUIBlueprintTools.inspect(bp))

editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
for map_name, mode in [('L_MainMenu', '/Script/PopStrike.PSMenuGameMode'), ('L_Lobby', '/Script/PopStrike.PSLobbyGameMode')]:
    shutil.copy2(project / 'Content/UI/Maps' / (map_name + '.umap'), backup / (map_name + '.umap'))
    assert level.load_level('/Game/UI/Maps/' + map_name)
    editor.get_editor_world().get_world_settings().set_editor_property('default_game_mode', unreal.load_class(None, mode))
    assert level.save_current_level()
(project / 'Saved/UIBuildReport.txt').write_text('\n'.join(reports), encoding='utf-8')
unreal.log('PS_UI_BUILD_OK: 3 blueprints compiled, 8 button events, 2 map game modes. Backup: ' + str(backup))
