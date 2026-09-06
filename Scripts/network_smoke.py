"""Local process smoke test; only terminates processes it creates. No installed server needed."""
from pathlib import Path
import subprocess
import time
import json
import argparse

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--engine', default='D:/UnrealEngine-release', help='Unreal Engine root directory')
exe = Path(parser.parse_args().engine) / 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
logs = root / 'Saved/Logs'
processes = []
results = {}

def start(name, args):
    log = logs / (name + '.log')
    if log.exists():
        log.rename(log.with_suffix('.previous.log')) if not log.with_suffix('.previous.log').exists() else log.unlink()
    proc = subprocess.Popen([str(exe), str(root / 'PopStrike.uproject'), *args,
        '-unattended', '-nullrhi', '-nosound', '-NoSplash', '-NoLoadingScreen',
        '-abslog=' + str(log)], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT,
        creationflags=subprocess.CREATE_NO_WINDOW)
    processes.append(proc)
    return proc, log

def wait_for(proc, log, text, seconds=75):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        data = log.read_text(encoding='utf-8', errors='replace') if log.exists() else ''
        if text in data: return data
        if proc.poll() is not None: raise RuntimeError(f'{log.name} exited {proc.returncode}: {data[-3000:]}')
        time.sleep(1)
    raise TimeoutError(f'{log.name}: missing {text}')

try:
    server, server_log = start('NetworkSmokeServer', ['/Game/UI/Maps/L_Lobby', '-server', '-port=17877'])
    wait_for(server, server_log, 'listening on port 17877')
    print('Dedicated server ready', flush=True)
    client, client_log = start('NetworkSmokeClient', ['127.0.0.1:17877', '-game'])
    wait_for(client, client_log, 'Welcomed by server')
    wait_for(client, client_log, 'Load map complete /Game/UI/Maps/L_Lobby')
    time.sleep(4)
    assert client.poll() is None
    results['client_connected_and_lobby_loaded'] = True
    print('Client connected and lobby loaded', flush=True)
    client2, client2_log = start('NetworkSmokeClientLag', ['127.0.0.1:17877', '-game', '-PktLag=150', '-PktLoss=5'])
    wait_for(client2, client2_log, 'Welcomed by server')
    wait_for(client2, client2_log, 'Load map complete /Game/UI/Maps/L_Lobby')
    time.sleep(4)
    results['second_client_lobby_loaded_with_network_simulation_flags'] = True
    print('Second client connected with lag/loss flags', flush=True)
    server.terminate(); server.wait(timeout=15)
    print('Server stopped; checking disconnect recovery', flush=True)
    for proc, log in [(client, client_log), (client2, client2_log)]:
        wait_for(proc, log, 'Load map complete /Game/UI/Maps/L_MainMenu', seconds=100)
        assert proc.poll() is None
        data = log.read_text(encoding='utf-8', errors='replace')
        assert 'Fatal error:' not in data and 'Accessed None' not in data
    results['both_clients_returned_to_menu_after_server_loss'] = True
    print('Both clients recovered to main menu', flush=True)
finally:
    for proc in processes:
        if proc.poll() is None:
            proc.terminate(); proc.wait(timeout=15)
    (root / 'Saved/NetworkSmokeResults.json').write_text(json.dumps(results, indent=2))
