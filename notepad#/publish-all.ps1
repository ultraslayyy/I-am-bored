param(
    [switch]$FrameworkDependent,
    [switch]$MakeInstaller
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
    if ($FrameworkDependent) {
        Write-Host "Publishing $pprofile (framework-dependent) into publish\$platformFolder..."
        dotnet publish /p:PublishProfile=$pprofile --self-contained false
    } else {
        Write-Host "Publishing $pprofile (self-contained) into publish\$platformFolder..."
        dotnet publish /p:PublishProfile=$pprofile
    }
    if ($MakeInstaller) {
        Write-Host "Building installer for $pprofile into installer/Output..."
        Set-Location ..\installer
        makensis /DPlatform=$platformFolder Installer.nsi
    }
}

Set-Location ..\