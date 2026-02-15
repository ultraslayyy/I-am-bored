// Using assembly file does not change file size (2048 bytes. EDIT: I just got 1536?), but uses 2 less bytes of data (without padding) so a win is a win. You can define NO_ASM (and not include .asm) to not use ASM
// gcc boy_windows.c boy_windows.asm -nostdlib -fno-asynchronous-unwind-tables -fno-unwind-tables -Wl,-entry,_start -Wl,-subsystem,console -Wl,--disable-reloc-section -fno-ident -Wl,--file-alignment,512 -lkernel32 -ffunction-sections -fdata-sections -Wl,--gc-sections -s -o boy_windows.exe
// strip boy_windows.exe
// objcopy --remove-section .comment boy_windows.exe
// OPTIONAL: python patch.py

typedef unsigned long DWORD;
typedef void* HANDLE;
typedef int BOOL;

#ifndef NO_ASM
extern char msg[];
#endif

__declspec(dllimport)
BOOL __stdcall WriteFile(
    HANDLE,
    const void*,
    DWORD,
    DWORD*,
    void*
);

__declspec(dllimport)
void __stdcall ExitProcess(unsigned int code);

void _start(void) {
    DWORD written;
    #ifdef NO_ASM
    char msg[] = "Hello, world!";
    #endif

    WriteFile(
        (HANDLE)-11,
        msg,
        14,
        &written,
        0
    );

    ExitProcess(0);
}