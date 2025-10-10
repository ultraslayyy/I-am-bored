@ECHO OFF

REM --- Config ---
set CERT_PATH=C:\smthn.pfx
set CERT_PASS=blahblahblah
set TIMESTAMP_URL=http://timestamp.digicert.com
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe"
REM --- End of Config ---

if "%1"=="" (
    echo Usage: %0 InstallerFile.exe
    exit /b 1
)

set FILE_TO_SIGN=%1

if not exist "%SIGNTOOL%" (
    echo Skipping signing: signtool not found at "%SIGNTOOL%"
    exit /b 0
)

echo Signing file: %FILE_TO_SIGN% ...
%SIGNTOOL% sign /f "%CERT_PATH%" /p "%CERT_PASS%" /tr "%TIMESTAMP_URL%" /td sha256 /fd sha256 "%FILE_TO_SIGN%"

if errorlevel 1 (
    echo Failed to sign file %FILE_TO_SIGN%
    exit /b 1
) else (
    echo File signed successfully: %FILE_TO_SIGN%
)

pause