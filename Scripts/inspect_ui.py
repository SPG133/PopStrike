import unreal
from pathlib import Path

out = Path(unreal.Paths.project_saved_dir()) / 'UIBlueprintReport.txt'
reports = []
for name in ('WBP_PlayerSlot', 'WBP_MainMenu', 'WBP_Lobby'):
    bp = unreal.load_asset('/Game/UI/Lobby/' + name)
    assert bp, name
    reports.append(name + '\n' + unreal.PSUIBlueprintTools.inspect(bp))
out.write_text('\n'.join(reports), encoding='utf-8')
unreal.log('PS_UI_INSPECT_OK')
