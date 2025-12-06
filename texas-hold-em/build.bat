@echo off
setlocal

rem %1 = build mode

if "%1"=="dll"      goto BuildDLL
if "%1"=="exe"      goto BuildEXE
if "%1"=="a"        goto BuildA
if "%1"=="a-exe"    goto BuildAEXE
if "%1"=="a-full"   goto BuildAFULL
if "%1"=="clean"    goto Clean

rem Default: build DLL + EXE
call :BuildDLL
call :BuildEXE
goto :eof


:BuildDLL
echo === Building DLL ===
g++ -c ultras_utils.cpp -DULTRAS_UTILS_EXPORTS -o ultras_utils.o -I.
g++ -shared -o ultras_utils.dll ultras_utils.o -Wl,--out-implib,ultras_utils.lib -lws2_32
del ultras_utils.o
goto :eof


:BuildEXE
echo === Building EXE (DLL version) ===
g++ -c main.cpp -o main.o -I. -ffunction-sections -fdata-sections
g++ main.o ultras_utils.lib -o texasholdem.exe -lws2_32 -Wl,--gc-sections
del main.o ultras_utils.lib
goto :eof


:BuildA
echo === Building static library (.a) ===
g++ -c ultras_utils.cpp -DULTRAS_UTILS_STATIC -o ultras_utils.o -I.
ar rcs ultras_utils.a ultras_utils.o
del ultras_utils.o
goto :eof


:BuildAEXE
echo === Building EXE (static .a version) ===
if not exist ultras_utils.a (
    echo ultras_utils.a not found — building it first...
    call :BuildA
)
g++ -c main.cpp -DULTRAS_UTILS_STATIC -o main.o -I.
g++ main.o ultras_utils.a -o texasholdem.exe -lws2_32
del ultras_utils.a main.o
goto :eof


:BuildAFULL
echo === Full static build (.a + exe) ===
call :BuildA
call :BuildAEXE
goto :eof


:Clean
echo === Cleaning build files ===
del /q texasholdem.exe 2>nul
del /q ultras_utils.dll 2>nul
del /q ultras_utils.lib 2>nul
del /q ultras_utils.a 2>nul
echo Done.
goto :eof
