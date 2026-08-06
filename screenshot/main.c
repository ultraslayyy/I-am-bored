// gcc -Os -s -nostdlib -mwindows -fdata-sections -ffunction-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -Wl,--gc-sections -Wl,--build-id=none -Wl,-e,mainCRTStartup main.c -o screenshot.exe -luser32 -lgdi32 -lkernel32

#include <windows.h>

BOOL SaveBitmapToFile(HBITMAP hBitmap, HDC hDC, const char *filename) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi = {0};
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth       = bmp.bmWidth;
    bi.biHeight      = bmp.bmHeight;
    bi.biPlanes      = 1;
    bi.biBitCount    = 32;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    bi.biSizeImage = dwBmpSize;

    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (!hDIB) return FALSE;

    char *lpbitmap = (char *)GlobalLock(hDIB);

    if (!GetDIBits(hDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS)) {
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return FALSE;
    }

    BITMAPFILEHEADER bfh = {0};
    bfh.bfType    = 0x4D42; // "BM"
    bfh.bfSize    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return FALSE;
    }

    DWORD dwWritten;
    WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hFile, lpbitmap, dwBmpSize, &dwWritten, NULL);

    CloseHandle(hFile);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    return TRUE;
}

int mainCRTStartup() {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    
    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap    = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY | CAPTUREBLT);

    SelectObject(hMemoryDC, hOldBitmap);

    SaveBitmapToFile(hBitmap, hMemoryDC, "screenshot.bmp");

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return 0;
}