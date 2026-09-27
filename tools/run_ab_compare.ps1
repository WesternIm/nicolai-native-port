param(
    [string]$ReferencePack = $env:NICOLAI_REFERENCE_PACK,
    [string]$VoiceDir = $env:NICOLAI_VOICE_DIR,
    [ValidateSet("m34-shared", "m34-unit", "m36", "m36-chain")]
    [string]$CandidateProfile = "m34-shared",
    [string]$BuildDir = "build-ab",
    [string]$OutputRoot = "",
    [string]$Python = "python",
    [switch]$SkipBuild,
    [switch]$NoOpen
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Fail([string]$message) {
    throw $message
}

function Resolve-ExistingDirectory([string]$value, [string]$label) {
    if ([string]::IsNullOrWhiteSpace($value)) { return $null }
    $resolved = Resolve-Path -LiteralPath $value -ErrorAction SilentlyContinue
    if (-not $resolved) { Fail "$label not found: $value" }
    $item = Get-Item -LiteralPath $resolved.Path
    if (-not $item.PSIsContainer) { Fail "$label is not a directory: $value" }
    return $item.FullName
}

function Find-ReferencePack {
    $roots = @(
        (Get-Location).Path,
        (Split-Path -Parent (Get-Location).Path),
        (Join-Path $HOME "Downloads"),
        (Join-Path $HOME "Desktop")
    ) | Where-Object { $_ -and (Test-Path -LiteralPath $_) }

    $candidates = @()
    foreach ($root in $roots) {
        $candidates += Get-ChildItem -LiteralPath $root -Directory -Filter "reference_pack_*" -ErrorAction SilentlyContinue |
            Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "manifest.json") }
    }
    $selected = $candidates | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($selected) { return $selected.FullName }
    return $null
}

function Find-VoiceDir {
    $roots = @(
        ${env:ProgramFiles(x86)},
        $env:ProgramFiles,
        (Get-Location).Path,
        (Split-Path -Parent (Get-Location).Path)
    ) | Where-Object { $_ -and (Test-Path -LiteralPath $_) }

    foreach ($root in $roots) {
        Write-Host "Searching voice assets under $root ..."
        $dat = Get-ChildItem -LiteralPath $root -File -Filter "nicolai16.dat" -Recurse -ErrorAction SilentlyContinue |
            Select-Object -First 1
        if ($dat) {
            $dir = $dat.Directory.FullName
            if ((Test-Path -LiteralPath (Join-Path $dir "exc_rus.txt")) -and
                (Test-Path -LiteralPath (Join-Path $dir "abb_rus.txt"))) {
                return $dir
            }
        }
    }
    return $null
}

function Invoke-Checked([string]$label, [scriptblock]$command) {
    Write-Host ""
    Write-Host "== $label =="
    & $command
    if ($LASTEXITCODE -ne 0) {
        Fail "$label failed with exit code $LASTEXITCODE"
    }
}

function Invoke-Renderer([string]$outDir, [string]$stdout, [string]$stderr) {
    # Windows PowerShell 5.1 wraps native diagnostic stderr as ErrorRecords.
    # WORDSTR/PHYSICAL are normal diagnostics, not a failed render. The native
    # exit code and downstream 22-file metric check decide success instead.
    $savedPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = "Continue"
        & $renderer $voiceDat $exc $abb $corpusPath $outDir 1> $stdout 2> $stderr
        $renderExit = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $savedPreference
    }
    if ($renderExit -ne 0) { Fail "Render failed ($renderExit); see $stderr" }
}

$experimentVars = @(
    "NICOLAI_STATEFUL_TDS",
    "NICOLAI_SHARED_PHONE_DURATION",
    "NICOLAI_PC_SEG_TIMELINE",
    "NICOLAI_SEARCH_JOIN_PHASE",
    "NICOLAI_M36_TRANSITION_EXECUTOR",
    "NICOLAI_M36_CHAIN_EXECUTOR"
)

function Clear-ExperimentEnvironment {
    foreach ($name in $experimentVars) {
        Remove-Item "Env:$name" -ErrorAction SilentlyContinue
    }
}

function Enable-CandidateProfile([string]$profile) {
    Clear-ExperimentEnvironment
    switch ($profile) {
        "m36-chain" {
            $env:NICOLAI_M36_CHAIN_EXECUTOR = "1"
        }
        "m34-unit" {
            $env:NICOLAI_STATEFUL_TDS = "1"
            $env:NICOLAI_SHARED_PHONE_DURATION = "0"
        }
        "m34-shared" {
            $env:NICOLAI_STATEFUL_TDS = "1"
            $env:NICOLAI_SHARED_PHONE_DURATION = "1"
        }
        "m36" {
            $batchSource = Join-Path (Get-Location).Path "tools\nicolai_batch_render.cpp"
            if (-not (Test-Path -LiteralPath $batchSource)) {
                Fail "Cannot verify M36 audio-path support: missing $batchSource"
            }
            $sourceText = Get-Content -LiteralPath $batchSource -Raw
            if ($sourceText -notmatch "NICOLAI_M36_TRANSITION_EXECUTOR") {
                Fail "Candidate profile 'm36' is intentionally blocked: the route-level M36 audio path is not wired into nicolai_batch_render yet. This prevents accidentally comparing stable WAVs against themselves. Use -CandidateProfile m34-shared for the current executable, or wire NICOLAI_M36_TRANSITION_EXECUTOR into synthesis first."
            }
            $env:NICOLAI_M36_TRANSITION_EXECUTOR = "1"
        }
    }
}

$repoRoot = (Get-Location).Path
$ReferencePack = Resolve-ExistingDirectory $ReferencePack "Reference pack"
if (-not $ReferencePack) {
    $ReferencePack = Find-ReferencePack
}
if (-not $ReferencePack) {
    Fail "Reference pack was not found. Extract reference_pack_... so it contains manifest.json, then either pass -ReferencePack C:\path\to\reference_pack or set NICOLAI_REFERENCE_PACK."
}
if (-not (Test-Path -LiteralPath (Join-Path $ReferencePack "manifest.json"))) {
    Fail "Reference pack has no manifest.json: $ReferencePack"
}

$VoiceDir = Resolve-ExistingDirectory $VoiceDir "Voice directory"
if (-not $VoiceDir) {
    $VoiceDir = Find-VoiceDir
}
if (-not $VoiceDir) {
    Fail "Nicolai voice directory was not found. Pass -VoiceDir C:\path\to\voice or set NICOLAI_VOICE_DIR. It must contain nicolai16.dat, exc_rus.txt and abb_rus.txt."
}

$voiceDat = Join-Path $VoiceDir "nicolai16.dat"
$exc = Join-Path $VoiceDir "exc_rus.txt"
$abb = Join-Path $VoiceDir "abb_rus.txt"
foreach ($required in @($voiceDat, $exc, $abb)) {
    if (-not (Test-Path -LiteralPath $required)) { Fail "Missing required voice asset: $required" }
}

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $OutputRoot = Join-Path $repoRoot "metrics-work\ab-$stamp"
} else {
    $OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
}
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$baselineDir = Join-Path $OutputRoot "baseline"
$candidateDir = Join-Path $OutputRoot "candidate"
$logsDir = Join-Path $OutputRoot "logs"
$cacheDir = Join-Path $OutputRoot "feature-cache"
New-Item -ItemType Directory -Force -Path $baselineDir, $candidateDir, $logsDir, $cacheDir | Out-Null
foreach ($renderDir in @($baselineDir, $candidateDir)) {
    if (@(Get-ChildItem -LiteralPath $renderDir -File -Filter "*.wav").Count -gt 0) {
        Fail "OutputRoot already contains rendered WAVs; choose a fresh directory so stale files cannot count as successful renders."
    }
}

$manifest = Get-Content -LiteralPath (Join-Path $ReferencePack "manifest.json") -Raw -Encoding UTF8 | ConvertFrom-Json
if (-not $manifest.results -or $manifest.results.Count -ne 22) {
    Fail "Expected a 22-phrase reference manifest; got $($manifest.results.Count)."
}
$corpusPath = Join-Path $OutputRoot "corpus.tsv"
$lines = foreach ($row in $manifest.results) { "{0}`t{1}" -f $row.id, $row.text }
[System.IO.File]::WriteAllLines($corpusPath, $lines, (New-Object System.Text.UTF8Encoding($false)))

if (-not $SkipBuild) {
    Invoke-Checked "Configure A/B build" {
        & cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=Release
    }
    Invoke-Checked "Build nicolai_batch_render" {
        & cmake --build $BuildDir --config Release --target nicolai_batch_render --parallel
    }
}

$exeCandidates = @(
    (Join-Path $repoRoot "$BuildDir\Release\nicolai_batch_render.exe"),
    (Join-Path $repoRoot "$BuildDir\nicolai_batch_render.exe"),
    (Join-Path $repoRoot "$BuildDir\Release\nicolai_batch_render"),
    (Join-Path $repoRoot "$BuildDir\nicolai_batch_render")
)
$renderer = $exeCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $renderer) {
    Fail "nicolai_batch_render was not found under $BuildDir after build."
}

Write-Host ""
Write-Host "Reference:  $ReferencePack"
Write-Host "Voice:      $VoiceDir"
Write-Host "Renderer:   $renderer"
Write-Host "Candidate:  $CandidateProfile"
Write-Host "Output:     $OutputRoot"

Clear-ExperimentEnvironment
$baselineStdout = Join-Path $logsDir "baseline.stdout.txt"
$baselineStderr = Join-Path $logsDir "baseline.stderr.txt"
Write-Host ""
Write-Host "== Render baseline stable =="
Invoke-Renderer $baselineDir $baselineStdout $baselineStderr

Enable-CandidateProfile $CandidateProfile
$candidateStdout = Join-Path $logsDir "candidate.stdout.txt"
$candidateStderr = Join-Path $logsDir "candidate.stderr.txt"
Write-Host ""
Write-Host "== Render candidate $CandidateProfile =="
Invoke-Renderer $candidateDir $candidateStdout $candidateStderr
Clear-ExperimentEnvironment

$changedWavs = 0
foreach ($row in $manifest.results) {
    $baseWav = Join-Path $baselineDir "$($row.id).wav"
    $candWav = Join-Path $candidateDir "$($row.id).wav"
    if (-not (Test-Path -LiteralPath $baseWav) -or
        -not (Test-Path -LiteralPath $candWav)) {
        Fail "Missing baseline/candidate WAV for $($row.id); inspect renderer logs."
    }
    if ((Get-FileHash -LiteralPath $baseWav).Hash -ne
        (Get-FileHash -LiteralPath $candWav).Hash) { $changedWavs++ }
}
if ($changedWavs -eq 0) { Fail "Candidate WAVs are all identical to baseline; experiment did not run." }
$crossPaths = 0
if ($CandidateProfile -eq "m36-chain") {
    foreach ($line in (Get-Content -LiteralPath $candidateStdout)) {
        if ($line -match '^M36\t') {
            $parts = $line -split "`t"
            $crossPaths += [int]$parts[3]
        }
    }
    if ($crossPaths -eq 0) { Fail "No exact M36 cross paths observed; check renderer build and logs." }
}

$baselineJson = Join-Path $OutputRoot "baseline-parity.json"
$baselineCsv = Join-Path $OutputRoot "baseline-parity.csv"
$candidateJson = Join-Path $OutputRoot "candidate-parity.json"
$candidateCsv = Join-Path $OutputRoot "candidate-parity.csv"
$compareJson = Join-Path $OutputRoot "comparison.json"
$compareCsv = Join-Path $OutputRoot "comparison.csv"

Invoke-Checked "Historical parity metrics: baseline" {
    & $Python tools/measure_parity.py $ReferencePack $baselineDir --json $baselineJson --csv $baselineCsv *> (Join-Path $logsDir "baseline-metrics.txt")
}
Invoke-Checked "Historical parity metrics: candidate" {
    & $Python tools/measure_parity.py $ReferencePack $candidateDir --json $candidateJson --csv $candidateCsv *> (Join-Path $logsDir "candidate-metrics.txt")
}
Invoke-Checked "Phrase-by-phrase A/B comparison" {
    & $Python tools/compare_parity.py $baselineJson $candidateJson --json $compareJson --csv $compareCsv *> (Join-Path $logsDir "comparison.txt")
}

$v2_2048 = Join-Path $OutputRoot "diagnostics-v2-2048.json"
$v2_1024 = Join-Path $OutputRoot "diagnostics-v2-1024.json"
Invoke-Checked "M35 diagnostics (2048 pitch frame)" {
    & $Python tools/measure_parity_v2.py $ReferencePack $OutputRoot --candidates baseline candidate --json $v2_2048 --cache $cacheDir --pitch-frame 2048 *> (Join-Path $logsDir "diagnostics-v2-2048.txt")
}
Invoke-Checked "M35 diagnostics (1024 pitch frame)" {
    & $Python tools/measure_parity_v2.py $ReferencePack $OutputRoot --candidates baseline candidate --json $v2_1024 --cache $cacheDir --pitch-frame 1024 *> (Join-Path $logsDir "diagnostics-v2-1024.txt")
}

$base = Get-Content -LiteralPath $baselineJson -Raw | ConvertFrom-Json
$cand = Get-Content -LiteralPath $candidateJson -Raw | ConvertFrom-Json
$cmp = Get-Content -LiteralPath $compareJson -Raw | ConvertFrom-Json

$metricNames = @(
    "mean_active_waveform_correlation",
    "active_duration_mae_percent",
    "total_duration_mae_percent",
    "f0_contour_mae_percent",
    "mean_mfcc_dtw",
    "rms_ratio_mae_percent"
)
$summaryRows = foreach ($name in $metricNames) {
    [pscustomobject]@{
        metric = $name
        baseline = [double]$base.summary.$name
        candidate = [double]$cand.summary.$name
        delta = [double]$cand.summary.$name - [double]$base.summary.$name
    }
}

$summaryObject = [ordered]@{
    schema = "nicolai-ab-comparison-v1"
    created_at = (Get-Date).ToString("o")
    candidate_profile = $CandidateProfile
    changed_wavs = $changedWavs
    exact_cross_paths = $crossPaths
    reference_pack = $ReferencePack
    voice_dir = $VoiceDir
    baseline_summary = $base.summary
    candidate_summary = $cand.summary
    headline = $summaryRows
    phrase_comparison = $cmp.metrics
    diagnostics_v2_2048 = (Split-Path -Leaf $v2_2048)
    diagnostics_v2_1024 = (Split-Path -Leaf $v2_1024)
}
$summaryJson = Join-Path $OutputRoot "summary.json"
$summaryObject | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $summaryJson -Encoding UTF8

$summaryTxt = Join-Path $OutputRoot "summary.txt"
$text = New-Object System.Collections.Generic.List[string]
$text.Add("Nicolai A/B comparison")
$text.Add("Candidate profile: $CandidateProfile")
$text.Add("Reference: $ReferencePack")
$text.Add("")
$text.Add(("{0,-38} {1,14} {2,14} {3,14}" -f "metric", "baseline", "candidate", "delta"))
$text.Add(("-" * 84))
foreach ($row in $summaryRows) {
    $text.Add(("{0,-38} {1,14:N6} {2,14:N6} {3,14:N6}" -f $row.metric, $row.baseline, $row.candidate, $row.delta))
}
$text.Add("")
$text.Add("Phrase win/regression counts are in comparison.json/csv.")
$text.Add("M35 shape/energy/pYIN/AC diagnostics are in diagnostics-v2-2048.json and diagnostics-v2-1024.json.")
$text.Add("Raw WAVs: baseline\ and candidate\. Renderer/metric stdout: logs\.")
[System.IO.File]::WriteAllLines($summaryTxt, $text, (New-Object System.Text.UTF8Encoding($false)))

Write-Host ""
Write-Host "============================================================"
Write-Host "A/B RESULTS"
Write-Host "============================================================"
Get-Content -LiteralPath $summaryTxt
Write-Host ""
Write-Host "Ready-to-compare bundle: $OutputRoot"

if (-not $NoOpen) {
    Start-Process explorer.exe -ArgumentList $OutputRoot | Out-Null
}
