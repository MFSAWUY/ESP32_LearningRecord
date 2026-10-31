<##
Build the handbook's ESP-IDF examples from real Windows paths.
Uses already installed packages, creates fresh caches under .tools/idf-current,
does not create drive mappings/junctions, and never flashes a device.
Examples:
  .\scripts\windows_idf.ps1 -Mode Check
  .\scripts\windows_idf.ps1 -Mode Build -Project gpio
  .\scripts\windows_idf.ps1 -Mode BuildAll -Parallel 2
  .\scripts\windows_idf.ps1 -Mode Ota
  .\scripts\windows_idf.ps1 -Mode OtaSelftestFail
This is the handbook's package-based environment, not a replacement for an
EIM-managed installation. Environment variables are restored after execution.
##>
[CmdletBinding()]
param(
    [ValidateSet('Check', 'Build', 'BuildAll', 'Ota', 'OtaSelftestFail')]
    [string]$Mode = 'Check',
    [ValidatePattern('^[a-z0-9_]+$')]
    [string]$Project = 'gpio',
    [ValidateSet(1, 2)]
    [int]$Parallel = 1,
    [switch]$RetryFailed
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$corePath = Join-Path $projectRoot '.tools/platformio'
$packagesPath = Join-Path $corePath 'packages'
$idfPath = Join-Path $packagesPath 'framework-espidf'
$pythonEnvironment = Join-Path $corePath 'penv/.espidf-6.1.0'
$pythonPath = Join-Path $pythonEnvironment 'Scripts/python.exe'
$idfScript = Join-Path $idfPath 'tools/idf.py'
$romPath = Join-Path $packagesPath 'tool-esp-rom-elfs'
$compilerPath = $null
foreach ($candidate in Get-ChildItem -LiteralPath $packagesPath -Directory) {
    if ($candidate.Name -notlike 'toolchain-xtensa-esp-elf*') { continue }
    $manifest = Join-Path $candidate.FullName 'package.json'
    if (-not (Test-Path -LiteralPath $manifest)) { continue }
    $metadata = Get-Content -LiteralPath $manifest -Raw | ConvertFrom-Json
    if ($metadata.version -eq '15.2.0+20251204') {
        $compilerPath = Join-Path $candidate.FullName 'bin'
        break
    }
}
if (-not $compilerPath) { throw 'ESP-IDF 6.1 requires the installed GCC 15.2.0+20251204 package.' }
$required = @($pythonPath, $idfScript, (Join-Path $compilerPath 'xtensa-esp32-elf-gcc.exe'),
    (Join-Path $packagesPath 'tool-cmake/bin/cmake.exe'),
    (Join-Path $packagesPath 'tool-ninja/ninja.exe'),
    (Join-Path $romPath 'esp32_rev0_rom.elf'))
foreach ($entry in $required) {
    if (-not (Test-Path -LiteralPath $entry)) { throw "Required installed tool is missing: $entry" }
}
if ((Get-Content -LiteralPath (Join-Path $idfPath 'version.txt') -Raw).Trim() -ne '6.1.0') {
    throw 'This launcher is pinned to ESP-IDF 6.1.0.'
}

$environment = @{
    IDF_PATH = $idfPath
    IDF_TOOLS_PATH = $corePath
    IDF_PYTHON_ENV_PATH = $pythonEnvironment
    ESP_IDF_VERSION = '6.1.0'
    PYTHONUTF8 = '1'
    IDF_PYTHON_CHECK_CONSTRAINTS = 'no'
    IDF_PY_BUILD_JOBS = '4'
    ESP_ROM_ELF_DIR = ($romPath.Replace('\', '/') + '/')
    CMAKE_NINJA_FORCE_RESPONSE_FILE = '1'
    PATH = (@($compilerPath, (Join-Path $packagesPath 'toolchain-esp32ulp/bin'),
        (Join-Path $packagesPath 'tool-cmake/bin'), (Join-Path $packagesPath 'tool-ninja'),
        (Join-Path $packagesPath 'tool-mconf'), (Join-Path $pythonEnvironment 'Scripts')) -join ';') + ';' + $env:PATH
}
$previous = @{}
$result = 0
try {
    foreach ($name in $environment.Keys) {
        $previous[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $environment[$name], 'Process')
    }
    if ($Mode -eq 'Check') {
        & $pythonPath $idfScript --version
        if ($LASTEXITCODE -ne 0) { throw 'idf.py version check failed.' }
        & (Join-Path $compilerPath 'xtensa-esp32-elf-gcc.exe') --version
        if ($LASTEXITCODE -ne 0) { throw 'GCC version check failed.' }
        & $pythonPath -m pip check
        if ($LASTEXITCODE -ne 0) { throw 'IDF Python dependency consistency check failed.' }
    } else {
        $flavor = if ($Mode -eq 'Ota') { 'ota-configured' } elseif ($Mode -eq 'OtaSelftestFail') { 'ota-selftest-fail' } else { 'default' }
        $workPath = Join-Path $projectRoot ".tools/idf-current/$flavor"
        $recordPath = Join-Path $projectRoot "validation/windows-repair/$flavor/build-windows.json"
        $buildArguments = @((Join-Path $PSScriptRoot 'build_idf_native.py'), '--work-root', $workPath,
            '--idf-path', $idfPath, '--python', $pythonPath, '--output', $recordPath,
            '--parallel', "$Parallel")
        if ($Mode -eq 'Build') { $buildArguments += @('--match', "/$Project") }
        if ($Mode -in @('Ota', 'OtaSelftestFail')) {
            $defaultsFile = if ($Mode -eq 'Ota') { 'validation/ota-build-only.defaults' } else { 'validation/windows-repair/ota-selftest-fail.defaults' }
            $buildArguments += @('--match', '/ota', '--defaults-overlay',
                (Join-Path $projectRoot $defaultsFile))
        }
        if ($RetryFailed) { $buildArguments += '--retry-failed' }
        & $pythonPath @buildArguments
        $result = $LASTEXITCODE
    }
} finally {
    foreach ($name in $previous.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previous[$name], 'Process')
    }
}
if ($result -ne 0) { throw "ESP-IDF build failed (exit $result). Inspect validation/windows-repair logs." }
