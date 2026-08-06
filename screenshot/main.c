// gcc -Os -s -nostdlib -mwindows -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fno-stack-protector -fomit-frame-pointer -Wl,--gc-sections -Wl,--build-id=none -Wl,-e,mainCRTStartup -Wl,--no-insert-timestamp -Wl,--file-alignment,512 -Wl,--section-alignment,512 -Wl,--no-seh -Wl,--disable-reloc-section main.c -o screenshot -luser32 -lgdi32 -lkernel32

#include <windows.h>

int mainCRTStartup(void) {
    int w  = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreen  = GetDC(0);
    HDC hMem     = CreateCompatibleDC(hScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, w, h);

    SelectObject(hMem, hBmp);
    BitBlt(hMem, 0, 0, w, h, hScreen, 0, 0, SRCCOPY | CAPTUREBLT);

    DWORD imgSize = w * h * 4;
    DWORD totalSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imgSize;

    void *buf = VirtualAlloc(0, totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    BITMAPFILEHEADER *bfh = (BITMAPFILEHEADER *)buf;
    BITMAPINFOHEADER *bi  = (BITMAPINFOHEADER *)(bfh + 1);
    char *pixels = (char *)(bi + 1);

    bfh->bfType    = 0x4D42; // "BM"
    bfh->bfSize    = totalSize;
    bfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    bi->biSize     = sizeof(BITMAPINFOHEADER);
    bi->biWidth    = w;
    bi->biHeight   = h;
    bi->biPlanes   = 1;
    bi->biBitCount = 32;

    GetDIBits(hMem, hBmp, 0, h, pixels, (BITMAPINFO *)bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFileA("screenshot.bmp", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    DWORD written;
    WriteFile(hFile, buf, totalSize, &written, 0);

    ExitProcess(0);
}