param(
    [string]$BuildDir = "build_m36_win32",
    [string]$OutputRoot = "metrics-work/m36/runtime-capture",
    [string]$Python = "python",
    [int]$ReadyTimeoutSeconds = 20,
    [int]$WarmupTimeoutSeconds = 30,
    [int]$TriggerTimeoutSeconds = 180,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$capture = $null
$stopFile = $null
Push-Location $repoRoot
try {
    $x86PowerShell = Join-Path $env:WINDIR "SysWOW64\WindowsPowerShell\v1.0\powershell.exe"
    if (-not (Test-Path -LiteralPath $x86PowerShell)) {
        throw "32-bit Windows PowerShell was not found: $x86PowerShell"
    }

    $outputDir = [System.IO.Path]::GetFullPath($OutputRoot)
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    $records = Join-Path $outputDir "phone-records.jsonl"
    $auditReport = Join-Path $outputDir "phone-records-audit.json"
    $captureStdout = Join-Path $outputDir "capture.stdout.log"
    $captureStderr = Join-Path $outputDir "capture.stderr.log"
    $stopFile = Join-Path $outputDir "capture.stop"
    Remove-Item -Force -ErrorAction SilentlyContinue `
        $records,$auditReport,$captureStdout,$captureStderr,$stopFile

    function Invoke-SapiTrigger(
        [string]$Label,
        [string]$TriggerOutputDir,
        [int]$TimeoutSeconds,
        [switch]$WarmupOnly
    ) {
        $triggerScript = Join-Path $PSScriptRoot "m36_sapi_trigger.ps1"
        $stdout = Join-Path $outputDir ($Label + ".stdout.log")
        $stderr = Join-Path $outputDir ($Label + ".stderr.log")
        $arguments = @(
            "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
            ('"' + $triggerScript + '"'),
            "-OutputDir", ('"' + $TriggerOutputDir + '"')
        )
        if ($WarmupOnly) { $arguments += "-WarmupOnly" }

        $process = Start-Process -FilePath $x86PowerShell -ArgumentList $arguments `
            -PassThru -WindowStyle Hidden -RedirectStandardOutput $stdout `
            -RedirectStandardError $stderr
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            try { $process.Kill() } catch { }
            throw "$Label timed out after $TimeoutSeconds seconds; see $stdout and $stderr"
        }
        if ($process.ExitCode -ne 0) {
            $detail = if (Test-Path $stderr) { Get-Content $stderr -Raw } else { "" }
            throw "$Label failed with exit code $($process.ExitCode): $detail"
        }
        if (Test-Path $stdout) { Get-Content $stdout }
    }

    # Warm up the original voice first. This starts/activates the out-of-process
    # Acapela engine so the debugger can attach to the process that owns mtsyc32.
    Invoke-SapiTrigger -Label "Nicolai warm-up" `
        -TriggerOutputDir (Join-Path $OutputRoot "warmup") `
        -TimeoutSeconds $WarmupTimeoutSeconds -WarmupOnly

    $engines = @(Get-Process -Name ettsengine -ErrorAction SilentlyContinue)
    if ($engines.Count -eq 0) {
        throw "No ettsengine.exe process found after Nicolai warm-up"
    }
    if ($engines.Count -gt 1) {
        $engines = @($engines | Sort-Object StartTime -Descending)
        Write-Warning "Multiple ettsengine processes found; using newest PID $($engines[0].Id)"
    }
    $engine = $engines[0]
    Write-Host "Using ettsengine PID $($engine.Id)"

    if (-not $SkipBuild) {
        & cmake -S . -B $BuildDir -A Win32 -DBUILD_TESTING=ON
        if ($LASTEXITCODE -ne 0) { throw "CMake Win32 configure failed" }
        & cmake --build $BuildDir --config Debug --target nicolai_m36_runtime_capture --parallel
        if ($LASTEXITCODE -ne 0) { throw "M36 runtime capture build failed" }
    }

    $captureExe = @(
        (Join-Path $BuildDir "Debug/nicolai_m36_runtime_capture.exe"),
        (Join-Path $BuildDir "nicolai_m36_runtime_capture.exe")
    ) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if (-not $captureExe) {
        throw "Capture executable was not found under $BuildDir"
    }

    $capture = Start-Process -FilePath $captureExe -ArgumentList @(
        [string]$engine.Id,
        ('"' + $records + '"'),
        ('"' + $stopFile + '"')
    ) -PassThru -WindowStyle Hidden -RedirectStandardOutput $captureStdout `
      -RedirectStandardError $captureStderr

    $deadline = (Get-Date).AddSeconds($ReadyTimeoutSeconds)
    $ready = $false
    while ((Get-Date) -lt $deadline) {
        if ($capture.HasExited) {
            $err = if (Test-Path $captureStderr) { Get-Content $captureStderr -Raw } else { "" }
            throw "Runtime capture exited before READY (code $($capture.ExitCode)): $err"
        }
        if (Test-Path $captureStdout) {
            $text = Get-Content $captureStdout -Raw -ErrorAction SilentlyContinue
            if ($text -match 'M36_RUNTIME_CAPTURE_READY') { $ready = $true; break }
        }
        Start-Sleep -Milliseconds 100
    }
    if (-not $ready) {
        New-Item -ItemType File -Force -Path $stopFile | Out-Null
        throw "Runtime capture did not become ready in $ReadyTimeoutSeconds seconds"
    }

    Write-Host "Capture ready; synthesizing canonical 22-phrase corpus"
    Invoke-SapiTrigger -Label "Canonical SAPI trigger" `
        -TriggerOutputDir (Join-Path $OutputRoot "trigger-wavs") `
        -TimeoutSeconds $TriggerTimeoutSeconds

    New-Item -ItemType File -Force -Path $stopFile | Out-Null
    if (-not $capture.WaitForExit(15000)) {
        try { $capture.Kill() } catch { }
        throw "Runtime capture did not detach after stop request"
    }
    if ($capture.ExitCode -ne 0) {
        $err = if (Test-Path $captureStderr) { Get-Content $captureStderr -Raw } else { "" }
        throw "Runtime capture failed (code $($capture.ExitCode)): $err"
    }

    $lastLine = (Get-Content $captureStdout | Select-Object -Last 1)
    Write-Host $lastLine
    if (-not (Test-Path $records)) { throw "Capture produced no record file" }
    $recordCount = @(Get-Content $records).Count
    if ($recordCount -le 0) { throw "Capture produced zero feature records" }

    Write-Host "Auditing captured original records against independent M36 replay"
    & $Python (Join-Path $PSScriptRoot "audit_phone_features_m36.py") `
        $records --output $auditReport
    if ($LASTEXITCODE -ne 0) {
        throw "M36 runtime phone-feature audit found mismatches; see $auditReport"
    }

    Write-Host "M36 runtime capture + audit complete: $recordCount records"
    Write-Host "Records: $records"
    Write-Host "Audit:   $auditReport"
} finally {
    if ($capture -and -not $capture.HasExited) {
        if ($stopFile) {
            try { New-Item -ItemType File -Force -Path $stopFile | Out-Null } catch { }
        }
        if (-not $capture.WaitForExit(5000)) {
            try { $capture.Kill() } catch { }
        }
    }
    Pop-Location
}
