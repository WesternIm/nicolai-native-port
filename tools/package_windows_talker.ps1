param(
    [string]$BuildDir = 'build-talker',
    [string]$OutputDir = 'out/Nicolai-Test-win32',
    [string]$ZipPath = 'out/Nicolai-Test-win32.zip'
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $repoRoot
try {
    $build = [IO.Path]::GetFullPath($BuildDir)
    $output = [IO.Path]::GetFullPath($OutputDir)
    $zip = [IO.Path]::GetFullPath($ZipPath)
    if ((Test-Path -LiteralPath $output) -or (Test-Path -LiteralPath $zip)) {
        throw 'Package output directory and ZIP must be fresh; no files were overwritten.'
    }
    $cache = Get-Content -LiteralPath (Join-Path $build 'CMakeCache.txt') -Raw
    if ($cache -notmatch 'NICOLAI_STATIC_RUNTIME:BOOL=ON') {
        throw 'Configure with -DNICOLAI_STATIC_RUNTIME=ON before packaging.'
    }
    $exe = Join-Path $build 'Release/NicolaiTalker.exe'
    if (-not (Test-Path -LiteralPath $exe)) {
        if ($cache -notmatch 'CMAKE_BUILD_TYPE:STRING=Release') {
            throw 'Only a Release executable may be packaged.'
        }
        $exe = Join-Path $build 'NicolaiTalker.exe'
    }
    if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw 'NicolaiTalker.exe was not built.' }
    $image = [IO.File]::ReadAllBytes($exe)
    $ntOffset = [BitConverter]::ToInt32($image, 0x3c)
    if ([BitConverter]::ToUInt16($image, $ntOffset + 4) -ne 0x14c) {
        throw 'Package must contain the x86 executable so original 32-bit SAPI is available.'
    }
    New-Item -ItemType Directory -Path $output | Out-Null
    Copy-Item -LiteralPath $exe -Destination (Join-Path $output 'NicolaiTalker.exe')
    Copy-Item -LiteralPath (Join-Path $repoRoot 'docs/WINDOWS_TEST_TALKER.txt') -Destination (Join-Path $output 'README.txt')
    $hash = (Get-FileHash -LiteralPath (Join-Path $output 'NicolaiTalker.exe') -Algorithm SHA256).Hash.ToLowerInvariant()
    $commit = (& git rev-parse HEAD).Trim()
    if ($LASTEXITCODE -ne 0) { throw 'Cannot determine source commit.' }
    $manifest = [ordered]@{
        format = 'nicolai-test-talker-package-v1'
        source_commit = $commit
        architecture = 'Windows x86 (also runs on x64)'
        configuration = 'Release, static MSVC runtime'
        exe_sha256 = $hash
        profiles = @('stable', 'm36-local', 'm36-chain', 'original-sapi')
        proprietary_inputs_included = $false
        original_sapi_engine_included = $false
        original_sapi_requires_installed_voice = $true
    } | ConvertTo-Json
    [IO.File]::WriteAllText((Join-Path $output 'build.json'), $manifest + "`n", [Text.UTF8Encoding]::new($false))
    $zipParent = Split-Path -Parent $zip
    if (-not (Test-Path -LiteralPath $zipParent)) { New-Item -ItemType Directory -Path $zipParent | Out-Null }
    Compress-Archive -LiteralPath @(
        (Join-Path $output 'NicolaiTalker.exe'),
        (Join-Path $output 'README.txt'),
        (Join-Path $output 'build.json')
    ) -DestinationPath $zip
    Write-Host "EXE: $(Join-Path $output 'NicolaiTalker.exe')"
    Write-Host "ZIP: $zip"
    Write-Host "SHA256: $hash"
} finally { Pop-Location }
