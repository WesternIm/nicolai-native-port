param(
    [Parameter(Mandatory=$true)][string]$BuildDir,
    [Parameter(Mandatory=$true)][string]$VoiceDataDir,
    [Parameter(Mandatory=$true)][string]$ReferencePack,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [Parameter(Mandatory=$true)][string]$Python,
    [switch]$Stateful,
    [switch]$SharedPhoneDuration,
    [double]$PhysicalLengthStrength=0.0,
    [double]$PhysicalEnergyStrength=0.0
)
$ErrorActionPreference="Stop"
$oldStateful=$env:NICOLAI_STATEFUL_TDS
$oldShared=$env:NICOLAI_SHARED_PHONE_DURATION
try {
    $env:NICOLAI_STATEFUL_TDS=[string][int]$Stateful.IsPresent
    $env:NICOLAI_SHARED_PHONE_DURATION=[string][int]$SharedPhoneDuration.IsPresent
    & (Join-Path $PSScriptRoot "run_m33_parity.ps1") -BuildDir $BuildDir -VoiceDataDir $VoiceDataDir -ReferencePack $ReferencePack -OutputDir $OutputDir -Python $Python -PhysicalLengthStrength $PhysicalLengthStrength -PhysicalEnergyStrength $PhysicalEnergyStrength
} finally {
    $env:NICOLAI_STATEFUL_TDS=$oldStateful
    $env:NICOLAI_SHARED_PHONE_DURATION=$oldShared
}
