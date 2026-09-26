param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$VoiceDataDir,
    [Parameter(Mandatory = $true)][string]$ReferencePack,
    [Parameter(Mandatory = $true)][string]$OutputDir,
    [Parameter(Mandatory = $true)][string]$Python,
    [switch]$PcSegTimeline,
    [switch]$NoPhaseSearch,
    [double]$PhysicalLengthStrength = 0.0,
    [double]$PhysicalEnergyStrength = 0.0
)
$ErrorActionPreference = "Stop"
$oldTimeline = $env:NICOLAI_PC_SEG_TIMELINE
$oldPhase = $env:NICOLAI_SEARCH_JOIN_PHASE
$oldLength = $env:NICOLAI_PHYSICAL_LENGTH_STRENGTH
$oldEnergy = $env:NICOLAI_PHYSICAL_ENERGY_STRENGTH
try {
    $env:NICOLAI_PC_SEG_TIMELINE = [string][int]$PcSegTimeline.IsPresent
    $env:NICOLAI_SEARCH_JOIN_PHASE = [string][int](-not $NoPhaseSearch.IsPresent)
    & (Join-Path $PSScriptRoot "run_m32_parity.ps1") `
        -BuildDir $BuildDir -VoiceDataDir $VoiceDataDir -ReferencePack $ReferencePack `
        -OutputDir $OutputDir -Python $Python `
        -PhysicalLengthStrength $PhysicalLengthStrength -PhysicalEnergyStrength $PhysicalEnergyStrength
} finally {
    $env:NICOLAI_PC_SEG_TIMELINE = $oldTimeline
    $env:NICOLAI_SEARCH_JOIN_PHASE = $oldPhase
    $env:NICOLAI_PHYSICAL_LENGTH_STRENGTH = $oldLength
    $env:NICOLAI_PHYSICAL_ENERGY_STRENGTH = $oldEnergy
}
