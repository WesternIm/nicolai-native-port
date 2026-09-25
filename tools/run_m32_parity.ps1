param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$VoiceDataDir,
    [Parameter(Mandatory = $true)][string]$ReferencePack,
    [Parameter(Mandatory = $true)][string]$OutputDir,
    [Parameter(Mandatory = $true)][string]$Python,
    [double]$PhysicalLengthStrength = 0.0,
    [double]$PhysicalEnergyStrength = 0.0
)

$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
$renderer = Join-Path $BuildDir "nicolai_batch_render.exe"
$database = Join-Path $VoiceDataDir "nicolai16.dat"
$exceptions = Join-Path $VoiceDataDir "exc_rus.txt"
$abbreviations = Join-Path $VoiceDataDir "abb_rus.txt"
$corpus = Join-Path $PSScriptRoot "parity_corpus_22.tsv"
$metrics = Join-Path $PSScriptRoot "measure_parity.py"

foreach ($path in @($renderer, $database, $exceptions, $abbreviations, $corpus, $metrics, $Python)) {
    if (-not (Test-Path -LiteralPath $path)) { throw "Missing required input: $path" }
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$env:NICOLAI_PHYSICAL_LENGTH_STRENGTH = [string]::Format(
    [Globalization.CultureInfo]::InvariantCulture, "{0:R}", $PhysicalLengthStrength)
$env:NICOLAI_PHYSICAL_ENERGY_STRENGTH = [string]::Format(
    [Globalization.CultureInfo]::InvariantCulture, "{0:R}", $PhysicalEnergyStrength)

& $renderer $database $exceptions $abbreviations $corpus $OutputDir
if ($LASTEXITCODE -ne 0) { throw "nicolai_batch_render failed with exit code $LASTEXITCODE" }

$rendered = @(Get-ChildItem -LiteralPath $OutputDir -Filter "*.wav" -File)
if ($rendered.Count -ne 22) { throw "Expected 22 rendered WAVs, found $($rendered.Count)" }

$json = Join-Path $OutputDir "parity.json"
$csv = Join-Path $OutputDir "parity.csv"
& $Python $metrics $ReferencePack $OutputDir --json $json --csv $csv
if ($LASTEXITCODE -ne 0) { throw "measure_parity.py failed with exit code $LASTEXITCODE" }

Write-Output "Rendered 22/22 WAVs"
Write-Output "Metrics: $json"
