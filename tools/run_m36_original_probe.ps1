param(
    [Parameter(Mandatory = $true)]
    [string]$Dll,

    [string]$BuildDir = "build_m36_win32",
    [string]$Output = "metrics-work/m36/m36-original-phone-probe.json"
)

$ErrorActionPreference = "Stop"
$expectedSha256 = "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7"

$dllPath = (Resolve-Path $Dll).Path
$actualSha256 = (Get-FileHash -Algorithm SHA256 $dllPath).Hash.ToLowerInvariant()
if ($actualSha256 -ne $expectedSha256) {
    throw "Unexpected mtsyc32.dll SHA256: $actualSha256"
}

Write-Host "Verified original mtsyc32.dll SHA256 $actualSha256"

& cmake -S . -B $BuildDir -G "Visual Studio 17 2022" -A Win32 -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

& cmake --build $BuildDir --config Debug --target nicolai_m36_phone_probe legacy_phone_features_test --parallel
if ($LASTEXITCODE -ne 0) { throw "Win32 M36 build failed" }

$unit = Join-Path $BuildDir "Debug/legacy_phone_features_test.exe"
$probe = Join-Path $BuildDir "Debug/nicolai_m36_phone_probe.exe"

& $unit
if ($LASTEXITCODE -ne 0) { throw "legacy_phone_features_test failed" }

$probeLine = (& $probe $dllPath | Select-Object -Last 1)
if ($LASTEXITCODE -ne 0) { throw "nicolai_m36_phone_probe failed" }

try {
    $result = $probeLine | ConvertFrom-Json
} catch {
    throw "Probe did not emit valid JSON: $probeLine"
}

if ($result.portable_cases -ne 259 -or $result.original_matches -ne 259) {
    throw "Original oracle mismatch: $probeLine"
}

$outputPath = [System.IO.Path]::GetFullPath($Output)
$outputDir = Split-Path -Parent $outputPath
if ($outputDir) { New-Item -ItemType Directory -Force -Path $outputDir | Out-Null }

$artifact = [ordered]@{
    schema = "nicolai-m36-original-phone-probe-v1"
    dll_sha256 = $actualSha256
    portable_cases = [int]$result.portable_cases
    original_matches = [int]$result.original_matches
    status = "pass"
}
$artifact | ConvertTo-Json | Set-Content -Encoding UTF8 $outputPath

Write-Host $probeLine
Write-Host "M36 original phone-feature oracle: PASS"
Write-Host "Saved $outputPath"
