param(
    [Parameter(Mandatory=$true)][string]$Exe,
    [Parameter(Mandatory=$true)][string]$TextFile,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [string]$VoiceDir = '',
    [switch]$Trace,
    [switch]$Analysis,
    [switch]$WindowsManagedLaunch,
    [switch]$Worker
)
# PRIVATE diagnostic artifacts only. This is not a SAPI installation repair.
$ErrorActionPreference = 'Stop'
if ($Analysis -and -not $Trace) { throw '-Analysis requires -Trace.' }
function Quote-Argument([string]$Value) {
    # CRT quoting also preserves backslashes immediately before a quote/end.
    '"' + [regex]::Replace([regex]::Replace($Value, '(\\*)"', '$1$1\"'), '(\\+)$', '$1$1') + '"'
}
function Invoke-OwnedRender {
    if (Get-CimInstance Win32_Process -Filter "Name='ettsengine.exe'" -OperationTimeoutSec 5) {
        throw 'Existing original servers are left untouched; close your original player first.'
    }
    $render = $null
    $ownedServer = $null
    $renderBirth = $null
    $exitCode = $null
    $failure = $null
    try {
        $mode = if ($Analysis) { '--render-analysis-trace' } elseif ($Trace) { '--render-trace' } else { '--render' }
        $arguments = @($mode, (Quote-Argument $VoiceDir), (Quote-Argument $TextFile),
            'original-sapi', (Quote-Argument (Join-Path $OutputDir 'result.wav')))
        $render = Start-Process -FilePath $Exe -ArgumentList $arguments -WorkingDirectory (Split-Path $Exe) `
            -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $OutputDir 'render.log') `
            -RedirectStandardError (Join-Path $OutputDir 'error.log')
        $render.Handle | Out-Null
        $renderBirth = $render.StartTime
        if (-not $render.WaitForExit(45000)) { throw 'Owned original render timeout; see render.log.' }
        # Flush redirected streams after the finite wait.
        $render.WaitForExit()
        $exitCode = $render.ExitCode
        if ($exitCode -ne 0) { throw 'Original render failed; see error.log and the capture worker log.' }
        $wav = Join-Path $OutputDir 'result.wav'
        if (-not (Test-Path -LiteralPath $wav) -or (Get-Item -LiteralPath $wav).Length -le 44) {
            throw 'Original render produced no audio.'
        }
    } catch { $failure = $_.Exception.Message }
    finally {
        if ($render) {
            if (-not $render.HasExited) { $render.Kill(); $render.WaitForExit() }
            # The SDK's autoexit server may outlive its completed render. Never
            # kill by name: bind the direct parent AND lifetime, retain a handle,
            # then re-check ownership before ending only this test's child.
            foreach ($candidate in @(Get-CimInstance Win32_Process -Filter "Name='ettsengine.exe'" -OperationTimeoutSec 5)) {
                if ($candidate.ParentProcessId -ne $render.Id) { continue }
                $ownedServer = Get-Process -Id $candidate.ProcessId -ErrorAction SilentlyContinue
                if (-not $ownedServer) { continue }
                try {
                    $ownedServer.Handle | Out-Null
                    $current = Get-CimInstance Win32_Process -Filter "ProcessId=$($ownedServer.Id)" -OperationTimeoutSec 5
                    if ($ownedServer.StartTime -ge $renderBirth -and $current -and
                        $current.ParentProcessId -eq $render.Id -and
                        [IO.Path]::GetFileName($ownedServer.Path) -ieq 'ettsengine.exe') {
                        if (-not $ownedServer.WaitForExit(1000)) { $ownedServer.Kill(); $ownedServer.WaitForExit() }
                    }
                } finally { $ownedServer.Dispose() }
            }
            $render.Dispose()
        }
        $wav = Join-Path $OutputDir 'result.wav'
        $report = [ordered]@{schema='nicolai-original-owned-probe-v1'; trace=[bool]$Trace;
            analysis=[bool]$Analysis;
            exit_code=$exitCode; success=($null -eq $failure); error=$failure;
            wav_bytes=0; wav_sha256=$null; proprietary_artifacts_private=$true}
        if (Test-Path -LiteralPath $wav) {
            $report.wav_bytes = (Get-Item -LiteralPath $wav).Length
            $report.wav_sha256 = (Get-FileHash -LiteralPath $wav -Algorithm SHA256).Hash.ToLowerInvariant()
        }
        [IO.File]::WriteAllText((Join-Path $OutputDir 'completion.json'),
            ($report | ConvertTo-Json) + "`n", [Text.UTF8Encoding]::new($false))
    }
    if ($failure) { throw $failure }
}
$Exe = [IO.Path]::GetFullPath($Exe)
$TextFile = [IO.Path]::GetFullPath($TextFile)
$OutputDir = [IO.Path]::GetFullPath($OutputDir)
if (-not (Test-Path -LiteralPath $Exe -PathType Leaf) -or
    -not (Test-Path -LiteralPath $TextFile -PathType Leaf)) { throw 'EXE and text input must exist.' }
if (-not $VoiceDir) { $VoiceDir = Split-Path $Exe }
$VoiceDir = [IO.Path]::GetFullPath($VoiceDir)
if ($Worker) {
    if (-not (Test-Path -LiteralPath $OutputDir -PathType Container)) { throw 'Worker directory must exist.' }
    foreach ($name in @('completion.json', 'result.wav', 'render.log', 'error.log',
        'linguistics-m46.jsonl', 'linguistics-m46-worker.log', 'linguistics-m46.stop')) {
        if (Test-Path -LiteralPath (Join-Path $OutputDir $name)) { throw 'Worker outputs must be fresh.' }
    }
    Invoke-OwnedRender
    exit 0
}
if (Test-Path -LiteralPath $OutputDir) { throw 'Output directory must be fresh; nothing was overwritten.' }
New-Item -ItemType Directory -Path $OutputDir | Out-Null
Copy-Item -LiteralPath $TextFile -Destination (Join-Path $OutputDir 'input.txt')
$TextFile = Join-Path $OutputDir 'input.txt'
if (-not $WindowsManagedLaunch) {
    Invoke-OwnedRender
} else {
    # Explicit local diagnostic alternative when an inherited launch context
    # hangs the legacy SDK. WMI uses normal Windows process creation; no job
    # limits, permissions, environment, installation or registry are changed.
    $hostExe = (Get-Process -Id $PID).Path
    $command = (Quote-Argument $hostExe) + ' -NoProfile -NonInteractive -File ' +
        (Quote-Argument $PSCommandPath) + ' -Worker -Exe ' + (Quote-Argument $Exe) +
        ' -TextFile ' + (Quote-Argument $TextFile) + ' -OutputDir ' + (Quote-Argument $OutputDir) +
        ' -VoiceDir ' + (Quote-Argument $VoiceDir)
    if ($Trace) { $command += ' -Trace' }
    if ($Analysis) { $command += ' -Analysis' }
    $startup = New-CimInstance -ClassName Win32_ProcessStartup -ClientOnly -Property @{ShowWindow=[uint16]0}
    $launchBirth = [datetime]::Now
    $launch = Invoke-CimMethod -ClassName Win32_Process -MethodName Create -OperationTimeoutSec 5 -Arguments @{
        CommandLine=$command; CurrentDirectory=(Split-Path $Exe); ProcessStartupInformation=$startup}
    if ($launch.ReturnValue -ne 0) { throw "Windows diagnostic launch failed: $($launch.ReturnValue)" }
    $helper = Get-Process -Id $launch.ProcessId -ErrorAction Stop
    try {
        $helper.Handle | Out-Null
        $identity = Get-CimInstance Win32_Process -Filter "ProcessId=$($helper.Id)" -OperationTimeoutSec 5
        if ($helper.Path -ine $hostExe -or $helper.StartTime -lt $launchBirth -or
            -not $identity -or $identity.CommandLine -cne $command) {
            throw 'Diagnostic helper image/lifetime/command mismatch.'
        }
        if (-not $helper.WaitForExit(55000)) { throw 'Diagnostic helper exceeded its own render deadline.' }
        $helper.WaitForExit()
        if ($helper.ExitCode -ne 0) { throw 'Diagnostic helper failed; inspect its private completion/log files.' }
    } finally { $helper.Dispose() }
}
$result = Get-Content -LiteralPath (Join-Path $OutputDir 'completion.json') -Raw | ConvertFrom-Json
if (-not $result.success) { throw 'Probe did not succeed; see private completion.json.' }
$result | ConvertTo-Json
