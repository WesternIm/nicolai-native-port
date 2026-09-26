param(
    [Parameter(Mandatory=$true)][string]$BuildDir,
    [Parameter(Mandatory=$true)][string]$VoiceDataDir,
    [Parameter(Mandatory=$true)][string]$ReferencePack,
    [Parameter(Mandatory=$true)][string]$OutputRoot,
    [Parameter(Mandatory=$true)][string]$Python,
    [switch]$SummarizeOnly
)
$ErrorActionPreference="Stop"
$cases=@(
    @{Name="baseline";Stateful=$false;Shared=$false;Length=0.0;Energy=0.0},
    @{Name="stateful-unit";Stateful=$true;Shared=$false;Length=0.0;Energy=0.0},
    @{Name="stateful-shared";Stateful=$true;Shared=$true;Length=0.0;Energy=0.0},
    @{Name="stateful-length";Stateful=$true;Shared=$true;Length=0.002;Energy=0.0},
    @{Name="stateful-energy";Stateful=$true;Shared=$true;Length=0.0;Energy=0.002},
    @{Name="stateful-both";Stateful=$true;Shared=$true;Length=0.002;Energy=0.002}
)
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
$rows=@()
foreach($case in $cases) {
    $output=Join-Path $OutputRoot $case.Name
    if(-not $SummarizeOnly) {
        & (Join-Path $PSScriptRoot "run_m34_parity.ps1") -BuildDir $BuildDir -VoiceDataDir $VoiceDataDir -ReferencePack $ReferencePack -OutputDir $output -Python $Python -Stateful:$case.Stateful -SharedPhoneDuration:$case.Shared -PhysicalLengthStrength $case.Length -PhysicalEnergyStrength $case.Energy
    }
    $report=Get-Content -LiteralPath (Join-Path $output "parity.json") -Raw | ConvertFrom-Json
    $rows += [pscustomobject]@{
        candidate=$case.Name
        stateful_tds=$case.Stateful
        shared_phone_duration=$case.Shared
        physical_length_strength=$case.Length
        physical_energy_strength=$case.Energy
        metrics=$report.summary
    }
}
$rows | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $OutputRoot "summary.json") -Encoding utf8
