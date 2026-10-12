<##
Runs an uncooked development server and two headless clients using the existing Editor build.
This checks connection only, not rendering, gameplay, target compilation or packaging.
##>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$EditorPath,
    [ValidateRange(1024, 65535)]
    [int]$Port = 17777,
    [ValidateRange(15, 300)]
    [int]$TimeoutSeconds = 90
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$projectFile = Join-Path $projectRoot 'PopStrike.uproject'
$editorFile = (Resolve-Path -LiteralPath $EditorPath).Path
$map = '/Game/Maps/Level_1/L_Lobby'
if (-not (Test-Path -LiteralPath (Join-Path $projectRoot 'Content/Maps/Level_1/L_Lobby.umap'))) {
    throw 'The existing lobby map is missing.'
}

$runId = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$logDirectory = Join-Path $projectRoot "Saved/NetworkBaseline/$runId"
New-Item -ItemType Directory -Path $logDirectory | Out-Null
$processes = [System.Collections.Generic.List[System.Diagnostics.Process]]::new()
$serverLog = Join-Path $logDirectory 'Server.log'
$clientLogs = @(1..2 | ForEach-Object { Join-Path $logDirectory "Client$_.log" })

function Read-RunLog([string]$Path) {
    if (Test-Path -LiteralPath $Path) {
        $stream = [IO.FileStream]::new(
            $Path,
            [IO.FileMode]::Open,
            [IO.FileAccess]::Read,
            [IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete)
        try {
            $reader = [IO.StreamReader]::new($stream)
            try { return $reader.ReadToEnd() } finally { $reader.Dispose() }
        } finally {
            $stream.Dispose()
        }
    }
    return ''
}

function Start-TestProcess([string[]]$ExtraArguments, [string]$LogPath) {
    # Headless, local-only diagnostics; no changes to project config or the user's editor.
    $arguments = @(
        ('"{0}"' -f $projectFile)
    ) + $ExtraArguments + @(
        '-unattended', '-nullrhi', '-nosound', '-nosplash', '-NoLiveCoding',
        '-DDC-ForceMemoryCache',
        '-DisablePlugins=ModelContextProtocol,EditorToolset',
        ('-abslog="{0}"' -f $LogPath)
    )
    $process = Start-Process -FilePath $editorFile -ArgumentList $arguments -PassThru -WindowStyle Hidden
    $processes.Add($process)
    return $process
}

function Wait-ForLog([string]$Path, [string]$Pattern, [System.Diagnostics.Process]$Process) {
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    do {
        if ($Process.HasExited) { throw "Process exited before ready: $Path (exit $($Process.ExitCode))" }
        if ((Read-RunLog $Path) -match $Pattern) { return }
        Start-Sleep -Milliseconds 250
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Timed out waiting for '$Pattern': $Path"
}

try {
    # Refuse to reuse someone else's UDP endpoint.
    $probe = [Net.Sockets.UdpClient]::new()
    try {
        $probe.ExclusiveAddressUse = $true
        $probe.Client.Bind([Net.IPEndPoint]::new([Net.IPAddress]::Loopback, $Port))
    } finally { $probe.Dispose() }

    Write-Output "Logs: $logDirectory"
    $server = Start-TestProcess @($map, '-server', "-port=$Port", '-MULTIHOME=127.0.0.1') $serverLog
    Wait-ForLog $serverLog "IpNetDriver listening on port $Port\b" $server
    $clients = @()
    foreach ($log in $clientLogs) {
        $clients += Start-TestProcess @("127.0.0.1:$Port", '-game') $log
    }
    for ($index = 0; $index -lt $clients.Count; $index++) {
        Wait-ForLog $clientLogs[$index] 'Welcomed by server' $clients[$index]
    }
    Wait-ForLog $serverLog '(?s)Join succeeded:.*Join succeeded:' $server
    foreach ($process in $processes) {
        if ($process.HasExited) { throw "Test process $($process.Id) exited unexpectedly." }
    }
    $serverText = Read-RunLog $serverLog
    if ($serverText -notmatch 'Game class is .?PSLobbyGameMode') {
        throw 'Connections succeeded, but the expected PSLobbyGameMode was not confirmed.'
    }
    Write-Output 'PASS: lobby server accepted two independent headless clients.'
    Write-Output 'Not tested: lobby UI, team changes, map travel, movement, visuals, weak networks or packaged builds.'
} finally {
    # Only terminate Process objects created by this invocation; never kill by application name.
    foreach ($process in $processes) {
        if (-not $process.HasExited) { Stop-Process -InputObject $process -Force -ErrorAction Continue }
        $process.Dispose()
    }
}
