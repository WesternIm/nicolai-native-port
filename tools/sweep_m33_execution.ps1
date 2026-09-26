param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$VoiceDataDir,
    [Parameter(Mandatory = $true)][string]$ReferencePack,
    [Parameter(Mandatory = $true)][string]$OutputRoot,
    [Parameter(Mandatory = $true)][string]$Python,
    [switch]$SummarizeOnly
)
$ErrorActionPreference = "Stop"
$cases = @(
    @{ Name = "baseline"; Timeline = $false; NoPhase = $false; Length = 0.0; Energy = 0.0 },
    @{ Name = "pc-timeline"; Timeline = $true; NoPhase = $false; Length = 0.0; Energy = 0.0 },
    @{ Name = "no-phase-search"; Timeline = $false; NoPhase = $true; Length = 0.0; Energy = 0.0 },
    @{ Name = "timeline-no-phase"; Timeline = $true; NoPhase = $true; Length = 0.0; Energy = 0.0 },
    @{ Name = "timeline-length"; Timeline = $true; NoPhase = $false; Length = 0.002; Energy = 0.0 },
    @{ Name = "timeline-energy"; Timeline = $true; NoPhase = $false; Length = 0.0; Energy = 0.002 },
    @{ Name = "timeline-both"; Timeline = $true; NoPhase = $false; Length = 0.002; Energy = 0.002 }
)
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
$rows = @()
foreach ($case in $cases) {
    $output = Join-Path $OutputRoot $case.Name
    if (-not $SummarizeOnly) {
        & (Join-Path $PSScriptRoot "run_m33_parity.ps1") `
            -BuildDir $BuildDir -VoiceDataDir $VoiceDataDir -ReferencePack $ReferencePack `
            -OutputDir $output -Python $Python -PcSegTimeline:$case.Timeline `
            -NoPhaseSearch:$case.NoPhase -PhysicalLengthStrength $case.Length `
            -PhysicalEnergyStrength $case.Energy
    }
    $report = Get-Content -LiteralPath (Join-Path $output "parity.json") -Raw | ConvertFrom-Json
    $rows += [pscustomobject]@{
        candidate = $case.Name
        pc_timeline = $case.Timeline
        phase_search = (-not $case.NoPhase)
        physical_length_strength = $case.Length
        physical_energy_strength = $case.Energy
        metrics = $report.summary
    }
}
$rows | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $OutputRoot "summary.json") -Encoding utf8
