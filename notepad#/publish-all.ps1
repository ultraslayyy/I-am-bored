param(
    [switch]$SingleFile,
    [switch]$FrameworkDependent,
    [switch]$MakeInstaller,
    [switch]$BothFramework
)

$originalDir = Get-Location
$publishDir = Join-Path -Path $originalDir -ChildPath "publish"

if (Test-Path $publishDir) {
    Remove-Item -Recurse -Force $publishDir
}

try {
    Set-Location src

    if ($BothFramework) {
        $profiles = @('Win64', 'Win86', 'WinArm64', 'Win64C', 'Win86C', 'WinArm64C')
    } elseif ($FrameworkDependent) {
        $profiles = @('Win64', 'Win86', 'WinArm64')
    } else {
        $profiles = @('Win64C', 'Win86C', 'WinArm64C')
    }

    foreach ($instProfile in $profiles) {

        switch ($instProfile) {
            "Win64"    { $platformFolder = "win-x64";  $pprofile = "Win64" }
            "Win64C"   { $platformFolder = "win-x64";  $pprofile = "Win64" }
            "Win86"    { $platformFolder = "win-x86";  $pprofile = "Win86" }
            "Win86C"   { $platformFolder = "win-x86";  $pprofile = "Win86" }
            "WinArm64" { $platformFolder = "win-arm64"; $pprofile = "WinArm64" }
            "WinArm64C"{ $platformFolder = "win-arm64"; $pprofile = "WinArm64" }
        }

        $publishArgs = @("/p:PublishProfile=$pprofile")

        if ($instProfile -like "*C") {
            $publishArgs += "/p:SelfContained=true"
        } else {
            $publishArgs += "/p:SelfContained=false"
        }

        if ($SingleFile) {
            $publishArgs += "/p:PublishSingleFile=true"
            if ($instProfile -like "*C") {
                $publishArgs += "/p:PublishTrimmed=true"
            }
        }

        $publishArgs += "/p:PublishReadyToRun=true"

        Write-Host "Publishing $pprofile ($($publishArgs -join ' ')) into publish\$platformFolder..."
        dotnet publish @publishArgs

        if ($MakeInstaller) {
            $installerPath = "..\installer"
            if (-not (Test-Path "$installerPath/build")) { 
                New-Item -ItemType Directory "$installerPath/build" | Out-Null 
            }

            Push-Location $installerPath
            Write-Host "Building installer for $pprofile into installer/Output..."

            if ($instProfile -like "*C") {
                makensis /D$pprofile /DCONTAINED Installer.nsi
            } else {
                makensis /D$pprofile Installer.nsi
            }

            Pop-Location
        }
    }
}
finally {
    Set-Location $originalDir
}