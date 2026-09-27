param(
    [switch]$WarmupOnly,
    [string]$OutputDir = "metrics-work/m36/sapi-trigger"
)

$ErrorActionPreference = "Stop"
if ([IntPtr]::Size -ne 4) {
    throw "m36_sapi_trigger.ps1 must run under 32-bit Windows PowerShell"
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$corpusPath = Join-Path $PSScriptRoot "parity_corpus_22.tsv"
if (-not (Test-Path -LiteralPath $corpusPath)) {
    throw "Missing corpus: $corpusPath"
}

$out = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $OutputDir))
New-Item -ItemType Directory -Force -Path $out | Out-Null

function Write-Stage([string]$stage) {
    [Console]::Out.WriteLine("M36_SAPI_STAGE $stage")
    [Console]::Out.Flush()
}

Write-Stage "create-voice"
$voice = New-Object -ComObject SAPI.SpVoice
Write-Stage "enumerate-tokens"
$tokens = @($voice.GetVoices())
$selected = $null
$selectedDescription = $null
foreach ($token in $tokens) {
    $description = [string]$token.GetDescription()
    if ($null -eq $selected -and
        ($description -match '(?i)nicolai|elan\s+tts\s+russian')) {
        $selected = $token
        $selectedDescription = $description
    }
}
if ($null -eq $selected) {
    throw "Nicolai was not found in 32-bit SAPI5"
}

Write-Stage "select-voice $selectedDescription"
$voice.Voice = $selected
Write-Stage "voice-selected"
Write-Stage "set-rate"
$voice.Rate = 0
Write-Stage "set-volume"
$voice.Volume = 100
Write-Stage "create-format"
$audioFormat = New-Object -ComObject SAPI.SpAudioFormat
$audioFormat.Type = 18 # SAFT16kHz16BitMono

$phrases = @()
if ($WarmupOnly) {
    # Windows PowerShell 5.1 reads BOM-less script literals in the ANSI code
    # page. Read the diagnostic phrase from the explicitly UTF-8 corpus instead.
    $firstRow = Get-Content -LiteralPath $corpusPath -Encoding UTF8 |
        Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Select-Object -First 1
    $firstParts = $firstRow -split "`t", 2
    if ($firstParts.Count -ne 2) { throw "Invalid warm-up corpus row" }
    $phrases = @([pscustomobject]@{ id = 'warmup'; text = $firstParts[1] })
} else {
    foreach ($line in Get-Content -LiteralPath $corpusPath -Encoding UTF8) {
        if ([string]::IsNullOrWhiteSpace($line)) { continue }
        $parts = $line -split "`t", 2
        if ($parts.Count -ne 2) { throw "Invalid corpus row: $line" }
        $phrases += [pscustomobject]@{ id = $parts[0]; text = $parts[1] }
    }
}

Write-Host "M36 SAPI trigger: $selectedDescription"
foreach ($item in $phrases) {
    $wav = Join-Path $out ($item.id + '.wav')
    $stream = New-Object -ComObject SAPI.SpFileStream
    try {
        $stream.Format = $audioFormat
        $stream.Open($wav, 3, $false) # SSFMCreateForWrite
        Write-Stage "set-stream $($item.id)"
        $voice.AudioOutputStream = $stream
        Write-Stage "speak $($item.id)"
        [void]$voice.Speak($item.text, 0)
        Write-Stage "spoken $($item.id)"
    } finally {
        try { $stream.Close() } catch { }
        [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($stream)
    }
    Write-Host "M36_SAPI $($item.id) $($item.text)"
}

try { [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($audioFormat) } catch { }
try { [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($voice) } catch { }

Write-Host "M36 SAPI trigger complete: $($phrases.Count) phrase(s)"
