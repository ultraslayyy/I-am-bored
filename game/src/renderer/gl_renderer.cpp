// Yes I'm aware this is OpenGL 1.1
// Reason is I don't want to make my own loader right now,
// As my last one failed
#ifdef USE_OPENGL
#include "gl_renderer.h"

bool GLRenderer::init(HWND hwnd, int w, int h) {
    width = w;
    height = h;

    hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

    hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    fontBase = glGenLists(256);
    HFONT font = CreateFontA(
        -20, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS, // OUT_TT_PRECIS
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, // ANTIALIASED_QUALITY
        DEFAULT_PITCH | FF_DONTCARE, // FF_DONTCARE
        "Segoe UI"
    );

    SelectObject(hdc, font);
    wglUseFontBitmapsA(hdc, 0, 256, fontBase);
    DeleteObject(font);

    return true;
}

void GLRenderer::clear(int r, int g, int b) {
    glClearColor(r / 255.0f, g / 255.0f,  b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLRenderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);

    glBegin(GL_QUADS);
        glVertex2f((float)x, (float)y);
        glVertex2f((float)(x + w), (float)y);
        glVertex2f((float)(x + w), (float)(y + h));
        glVertex2f((float)x, (float)(y + h));
    glEnd();
}

void GLRenderer::drawText(const char* text, float x, float y, float size, int r, int g, int b) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);

    float ascent = 20.0f; // Text size

    glRasterPos2f(0, 0);
    glBitmap(0, 0, 0, 0, x, -(y + ascent), nullptr);

    glListBase(fontBase);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GLRenderer::present() {
    SwapBuffers(hdc);
}

void GLRenderer::resize(int w, int h) {
    width = w;
    height = h;

    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

GLRenderer::~GLRenderer() {
    if (fontBase) glDeleteLists(fontBase, 256);
    
    wglMakeCurrent(NULL, NULL);
    if (hglrc) wglDeleteContext(hglrc);
    if (hdc) ReleaseDC(WindowFromDC(hdc), hdc);
}
#endif