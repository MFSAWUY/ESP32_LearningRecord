param(
    [string]$Python = 'python',
    [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._@-]*$')]
    [string]$HostAlias = 'lab-linux',
    [ValidatePattern('^(/[A-Za-z0-9._-]+)+$')]
    [string]$RemoteHandbook = '/home/pc/wty/ESP32/handbook',
    [ValidatePattern('^(/[A-Za-z0-9._-]+)+$')]
    [string]$RemoteTest = '/home/pc/wty/ESP32/test'
)

# Copies source files only. Save remote edits before replacing matching files.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$archivePath = Join-Path $projectRoot '.tools/source-sync.tar.gz'
& $Python (Join-Path $PSScriptRoot 'package_sources.py') --output $archivePath
if ($LASTEXITCODE -ne 0) { throw 'Packaging failed; nothing transferred.' }
$digest = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash.ToLowerInvariant()
& scp -o ConnectTimeout=10 $archivePath "${HostAlias}:$RemoteTest/source-sync.tar.gz"
if ($LASTEXITCODE -ne 0) { throw 'SCP failed; extraction was not attempted.' }

# Check the archive hash before extraction, then check every source file.
$remoteCommand = 'set -e; test "$(sha256sum {0}/source-sync.tar.gz | cut -d " " -f 1)" = "{1}"; tar -xzf {0}/source-sync.tar.gz -C {2}; cd {2}; {0}/desktop-venv/bin/python scripts/verify_manifest.py; {0}/desktop-venv/bin/python scripts/check_handbook.py' -f $RemoteTest, $digest, $RemoteHandbook
& ssh -o ConnectTimeout=10 $HostAlias $remoteCommand
if ($LASTEXITCODE -ne 0) { throw 'Remote extraction or verification failed; inspect output.' }
Write-Output 'Source archive transferred and verified successfully.'
