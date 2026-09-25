param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$VoiceDataDir,
    [Parameter(Mandatory = $true)][string]$ReferencePack,
    [Parameter(Mandatory = $true)][string]$OutputRoot,
    [Parameter(Mandatory = $true)][string]$Python,
    [double[]]$Strengths = @(0.0625, 0.125, 0.25, 0.5, 1.0),
    [ValidateSet("length", "energy", "both")]
    [string[]]$Modes = @("length", "energy", "both"),
    [switch]$SummarizeOnly
)

$ErrorActionPreference = "Stop"
$runner = Join-Path $PSScriptRoot "run_m32_parity.ps1"
$candidates = @(@{ Name = "baseline"; Length = 0.0; Energy = 0.0 })
foreach ($strength in $Strengths) {
    $tag = [string]::Format([Globalization.CultureInfo]::InvariantCulture, "{0:g}", $strength)
    if ($Modes -contains "length") {
        $candidates += @{ Name = "length-$tag"; Length = $strength; Energy = 0.0 }
    }
    if ($Modes -contains "energy") {
        $candidates += @{ Name = "energy-$tag"; Length = 0.0; Energy = $strength }
    }
    if ($Modes -contains "both") {
        $candidates += @{ Name = "both-$tag"; Length = $strength; Energy = $strength }
    }
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
$rows = @()
foreach ($candidate in $candidates) {
    $output = Join-Path $OutputRoot $candidate.Name
    if (-not $SummarizeOnly) {
        & $runner -BuildDir $BuildDir -VoiceDataDir $VoiceDataDir `
            -ReferencePack $ReferencePack -OutputDir $output -Python $Python `
            -PhysicalLengthStrength $candidate.Length `
            -PhysicalEnergyStrength $candidate.Energy
    }
    if (-not (Test-Path -LiteralPath (Join-Path $output "parity.json"))) {
        throw "Missing candidate parity report: $output"
    }
    $result = Get-Content -LiteralPath (Join-Path $output "parity.json") -Raw | ConvertFrom-Json
    $summary = $result.summary
    $rows += [pscustomobject]@{
        candidate = $candidate.Name
        physical_length_strength = $candidate.Length
        physical_energy_strength = $candidate.Energy
        rendered = $summary.rendered
        active_correlation = $summary.mean_active_waveform_correlation
        active_duration_ratio = $summary.mean_active_duration_ratio
        active_duration_mae_percent = $summary.active_duration_mae_percent
        total_duration_mae_percent = $summary.total_duration_mae_percent
        f0_contour_mae_percent = $summary.f0_contour_mae_percent
        mfcc_dtw = $summary.mean_mfcc_dtw
        rms_ratio = $summary.mean_rms_ratio
        rms_ratio_mae_percent = $summary.rms_ratio_mae_percent
    }
}

$rows | Export-Csv -LiteralPath (Join-Path $OutputRoot "sweep-summary.csv") `
    -NoTypeInformation -Encoding utf8BOM
$rows | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath `
    (Join-Path $OutputRoot "sweep-summary.json") -Encoding utf8
$rows | Sort-Object active_correlation -Descending | Format-Table -AutoSize
