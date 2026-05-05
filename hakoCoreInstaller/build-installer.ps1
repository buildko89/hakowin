param(
  [ValidateSet("Debug", "Release")]
  [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$launcherProject = Join-Path $root "SetupLauncher\SetupLauncher.csproj"
$launcherExe = Join-Path $root "SetupLauncher\bin\$Configuration\setup.exe"
$targetSetupExe = Join-Path $root "hakocore-win\$Configuration\setup.exe"
$targetMsi = Join-Path $root "hakocore-win\$Configuration\hakocore-win.msi"

if (-not (Test-Path -LiteralPath $targetMsi)) {
  throw "MSI was not found: $targetMsi`nBuild the Visual Studio setup project first, then run this script."
}

msbuild $launcherProject /p:Configuration=$Configuration /v:minimal
Copy-Item -LiteralPath $launcherExe -Destination $targetSetupExe -Force

Write-Host "Replaced bootstrapper: $targetSetupExe"
