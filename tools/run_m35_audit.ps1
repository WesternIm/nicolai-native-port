param(
    [Parameter(Mandatory=$true)][string]$BuildDir,
    [Parameter(Mandatory=$true)][string]$VoiceDataDir,
    [Parameter(Mandatory=$true)][string]$ReferencePack,
    [Parameter(Mandatory=$true)][string]$OutputRoot,
    [Parameter(Mandatory=$true)][string]$Python,
    [string]$PriorM34Root,
    [switch]$MetricsOnly
)
$ErrorActionPreference="Stop"
$execution=Join-Path $OutputRoot "execution"
$cache=Join-Path $OutputRoot "feature-cache"
& $Python (Join-Path $PSScriptRoot "test_m35_metrics.py")
if($LASTEXITCODE -ne 0) { throw "Voice-free metric contracts failed" }
if(-not $MetricsOnly) {
    & (Join-Path $PSScriptRoot "sweep_m34_execution.ps1") -BuildDir $BuildDir -VoiceDataDir $VoiceDataDir -ReferencePack $ReferencePack -OutputRoot $execution -Python $Python
}
$auditArgs=@($ReferencePack,$execution,"--json",(Join-Path $OutputRoot "clock-audit.json"))
if($PriorM34Root) { $auditArgs += @("--prior-root",$PriorM34Root) }
& $Python (Join-Path $PSScriptRoot "audit_clock_m35.py") @auditArgs
if($LASTEXITCODE -ne 0) { throw "Clock contracts failed" }
foreach($frame in @(1024,2048)) {
    & $Python (Join-Path $PSScriptRoot "measure_parity_v2.py") $ReferencePack $execution --candidates baseline stateful-unit stateful-shared stateful-length stateful-energy stateful-both --pitch-frame $frame --cache $cache --json (Join-Path $OutputRoot "parity-v2-$frame.json")
    if($LASTEXITCODE -ne 0) { throw "Metric calibration/evaluation failed at frame=$frame" }
}
& $Python (Join-Path $PSScriptRoot "write_parity_manifest.py") $VoiceDataDir $ReferencePack --json (Join-Path $OutputRoot "run-manifest.json")
if($LASTEXITCODE -ne 0) { throw "Manifest creation failed" }
