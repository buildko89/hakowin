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

# Locate MSBuild.exe dynamically
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = $null
if (Test-Path -LiteralPath $vswhere) {
  $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
}

if (-not $msbuild -or -not (Test-Path -LiteralPath $msbuild)) {
  $msbuildLoc = Get-Command msbuild -ErrorAction SilentlyContinue
  if ($msbuildLoc) {
    $msbuild = $msbuildLoc.Source
  } else {
    $paths = @(
      "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
      "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
      "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
      "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    )
    foreach ($path in $paths) {
      if (Test-Path -LiteralPath $path) {
        $msbuild = $path
        break
      }
    }
  }
}

if (-not $msbuild) {
  throw "MSBuild.exe could not be found. Please ensure Visual Studio or Build Tools are installed."
}

& $msbuild $launcherProject /p:Configuration=$Configuration /v:minimal
Copy-Item -LiteralPath $launcherExe -Destination $targetSetupExe -Force

Write-Host "Replaced bootstrapper: $targetSetupExe"

