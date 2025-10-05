# Notepad#
A remake of [Notepad++](https://notepad-plus-plus.org/) but in C# (get it because Notepad**++** is made in C**++**, so C**#** = Notepad**#**. Very funny).
This is Windows only (like Notepad++).

## Prerequisites
- [**.NET-9**](<https://dotnet.microsoft.com/en-us/download/dotnet/9.0#:~:text=like%20Visual%20Studio).-,SDK%209.0.305,-Downloads%20for%20.NET>)
- *Optional:* [**Inno Setup Compiler**](https://jrsoftware.org/isdl.php#:~:text=Filename-,Download%20Sites,-Downloads) *(if you're wishing to build the installer)*
- Windows

## Running
To run the program it's pretty simple. Run:
```bash
dotnet run
```
in the [src](src/) folder;

## Building
To build, run this command with the [src](src/) folder as the current working directory:
> [!IMPORTANT]
> Remove `--self-contained true` from this command if you don't want to bundle .NET inside it. Drastically reduces file size, but requires user running to have .NET installed.
```bash
dotnet publish -c Release -r win-x64 --self-contained true -p:PublishSingleFile=false -o ../publish/win-x64
```
To run the built app, just go into [publish/win-x64](publish/win-x64/) and run `notepad#.exe`.

### Building installer
To build the installer, you need the `Inno Setup Compiler` (check prerequisites). Once installed, just run (inside the [installer](installer/) directory):
```bash
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" Installer.iss
```