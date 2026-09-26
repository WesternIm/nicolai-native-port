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

$voice = New-Object -ComObject SAPI.SpVoice
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

$voice.Voice = $selected
$voice.Rate = 0
$voice.Volume = 100
$audioFormat = New-Object -ComObject SAPI.SpAudioFormat
$audioFormat.Type = 18 # SAFT16kHz16BitMono

$phrases = @()
if ($WarmupOnly) {
    $phrases = @([pscustomobject]@{ id = 'warmup'; text = 'мама' })
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
        $voice.AudioOutputStream = $stream
        [void]$voice.Speak($item.text, 0)
    } finally {
        try { $stream.Close() } catch { }
        [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($stream)
    }
    Write-Host "M36_SAPI $($item.id) $($item.text)"
}

try { [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($audioFormat) } catch { }
try { [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($voice) } catch { }

Write-Host "M36 SAPI trigger complete: $($phrases.Count) phrase(s)"
