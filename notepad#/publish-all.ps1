param(
    [switch]$SingleFile,
    [switch]$FrameworkDependent,
    [switch]$MakeInstaller,
    [switch]$FINAL
)

Set-Location src

$profiles = @('Win64', 'Win86', 'WinArm64')

foreach ($pprofile in $profiles) {
    switch ($pprofile) {
        "Win64"    { $platformFolder = "win-x64" }
        "Win86"    { $platformFolder = "win-x86" }
        "WinArm64" { $platformFolder = "win-arm64" }
        default    { $platformFolder = $pprofile }
    }
    
    $publicArgs = @("/p:PublishProfile=$pprofile")
    if ($FrameworkDependent) {
       $publishArgs += "/p:SelfContained=false"
    } else {
        $publishArgs += "/p:SelfContained=true"
    }
    if ($SingleFile) {
        $publishArgs += "/p:PublishSingleFile=true"
        if (-not $FrameworkDependent) { $publishArgs += "/p:PublishTrimmed=true" }
    }
    $publishArgs += "/p:PublishReadyToRun=true"
    Write-Host "Publishing $pprofile ($($publishArgs -join ', ')) into publish\$platformFolder..."
    dotnet publish $publishArgs

    if ($MakeInstaller) {
        $installerPath = "..\installer"
        if (-not (Test-Path "$installerPath/build")) { New-Item -ItemType Directory "$installerPath/build" }
        Push-Location ..\installer
        if ($FINAL) {
            Write-Host "Building installer for $pprofile into installer/Output (FINAL)..."
            makensis /DPlatform=$platformFolder /DFINAL Installer.nsi
        } else {
            Write-Host "Building installer for $pprofile into installer/Output..."
            makensis /DPlatform=$platformFolder Installer.nsi
        }
        Pop-Location
    }
}

Set-Location ..\