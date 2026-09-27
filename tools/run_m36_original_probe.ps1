param(
    [string]$Dll = "",

    [string]$BuildDir = "build_m36_win32",
    [string]$Output = "metrics-work/m36/m36-original-phone-probe.json",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$expectedSha256 = "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7"

function Resolve-NicolaiDll([string]$ExplicitPath) {
    $candidates = New-Object System.Collections.Generic.List[string]
    if ($ExplicitPath) { $candidates.Add($ExplicitPath) }

    if (${env:ProgramFiles(x86)}) {
        $candidates.Add((Join-Path ${env:ProgramFiles(x86)} "Elan\mtsyc32.dll"))
        $candidates.Add((Join-Path ${env:ProgramFiles(x86)} "Acapela\mtsyc32.dll"))
    }
    if ($env:ProgramFiles) {
        $candidates.Add((Join-Path $env:ProgramFiles "Elan\mtsyc32.dll"))
        $candidates.Add((Join-Path $env:ProgramFiles "Acapela\mtsyc32.dll"))
    }

    # The exact filename matters more than the install-root spelling. Keep the
    # recursive fallback bounded to the two Program Files roots and accept a
    # candidate only after the historical SHA256 has matched.
    foreach ($root in @(${env:ProgramFiles(x86)}, $env:ProgramFiles)) {
        if (-not $root -or -not (Test-Path $root)) { continue }
        Get-ChildItem -Path $root -Filter "mtsyc32.dll" -File -Recurse -ErrorAction SilentlyContinue |
            ForEach-Object { $candidates.Add($_.FullName) }
    }

    $seen = @{}
    foreach ($candidate in $candidates) {
        if (-not $candidate) { continue }
        try { $resolved = (Resolve-Path $candidate -ErrorAction Stop).Path } catch { continue }
        if ($seen.ContainsKey($resolved)) { continue }
        $seen[$resolved] = $true
        $sha = (Get-FileHash -Algorithm SHA256 $resolved).Hash.ToLowerInvariant()
        if ($sha -eq $expectedSha256) {
            return $resolved
        }
    }

    if ($ExplicitPath) {
        throw "Specified DLL was not the supported Nicolai mtsyc32.dll (expected SHA256 $expectedSha256)"
    }
    throw "Could not find the supported Nicolai mtsyc32.dll automatically. Re-run with -Dll C:\\path\\to\\mtsyc32.dll"
}

$dllPath = Resolve-NicolaiDll $Dll
$actualSha256 = (Get-FileHash -Algorithm SHA256 $dllPath).Hash.ToLowerInvariant()
Write-Host "Verified original mtsyc32.dll: $dllPath"
Write-Host "SHA256 $actualSha256"

# Do not hardcode a Visual Studio generator version. GitHub and local systems
# may have VS 2022, VS 2026 or newer; CMake selects the installed default while
# -A Win32 requests the required x86 target architecture.
if (-not $SkipBuild) {
    & cmake -S . -B $BuildDir -A Win32 -DBUILD_TESTING=ON
    if ($LASTEXITCODE -ne 0) { throw "CMake Win32 configure failed" }

    & cmake --build $BuildDir --config Debug --target nicolai_m36_phone_probe legacy_phone_features_test --parallel
    if ($LASTEXITCODE -ne 0) { throw "Win32 M36 build failed" }
}

$unit = @(
    (Join-Path $BuildDir "Debug/legacy_phone_features_test.exe"),
    (Join-Path $BuildDir "legacy_phone_features_test.exe")
) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
$probe = @(
    (Join-Path $BuildDir "Debug/nicolai_m36_phone_probe.exe"),
    (Join-Path $BuildDir "nicolai_m36_phone_probe.exe")
) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $unit -or -not $probe) {
    throw "M36 probe executables were not found under $BuildDir"
}

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
    dll_path = $dllPath
    dll_sha256 = $actualSha256
    portable_cases = [int]$result.portable_cases
    original_matches = [int]$result.original_matches
    status = "pass"
}
$artifact | ConvertTo-Json | Set-Content -Encoding UTF8 $outputPath

Write-Host $probeLine
Write-Host "M36 original phone-feature oracle: PASS"
Write-Host "Saved $outputPath"
