param(
    [Parameter(Mandatory=$true)][string]$Text,
    [Parameter(Mandatory=$true)][string]$OutWav,
    [string]$VoiceMatch = "Nicolai",
    [int]$Rate = 0,
    [int]$Volume = 100
)

$ErrorActionPreference = "Stop"

$voice = New-Object -ComObject SAPI.SpVoice
$tokens = $voice.GetVoices()
$selected = $null

for ($i = 0; $i -lt $tokens.Count; $i++) {
    $token = $tokens.Item($i)
    $desc = $token.GetDescription()
    if ($desc -match [regex]::Escape($VoiceMatch)) {
        $selected = $token
        break
    }
}

if ($null -eq $selected) {
    $available = @()
    for ($i = 0; $i -lt $tokens.Count; $i++) {
        $available += $tokens.Item($i).GetDescription()
    }
    throw "Voice '$VoiceMatch' not found. Available: $($available -join '; ')"
}

$voice.Voice = $selected
$voice.Rate = $Rate
$voice.Volume = $Volume

# Microsoft SAPI SpeechAudioFormatType: SAFT16kHz16BitMono = 18.
# Nicolai's native database is 16 kHz, so this avoids an extra comparison-time
# resampling step when SAPI accepts the format.
$format = New-Object -ComObject SAPI.SpAudioFormat
$format.Type = 18

$stream = New-Object -ComObject SAPI.SpFileStream
$stream.Format = $format
# SSFMCreateForWrite = 3
$stream.Open($OutWav, 3, $false)
try {
    $voice.AudioOutputStream = $stream
    # SVSFDefault = 0
    [void]$voice.Speak($Text, 0)
} finally {
    $stream.Close()
}

Write-Output "voice=$($selected.GetDescription())"
Write-Output "text=$Text"
Write-Output "wav=$OutWav"
Write-Output "format=PCM16 mono 16000Hz (SAPI type 18)"
