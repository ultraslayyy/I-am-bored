#ifdef USE_OPENGL
#include "gl_renderer.h"
#ifdef USE_OPENGL_33
#include <GL/wglext.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "./vendor/stb_truetype.h"
#include <fstream>
#include <vector>

static const char* fontPath = "./vendor/segoe-ui.ttf";
static const int FONT_ATLAS_SIZE = 512;
static const int FONT_ASCII_START = 32;
static const int FONT_ASCII_COUNT = 95;

static stbtt_bakedchar cdata[FONT_ASCII_COUNT];
static GLuint fontTex = 0;

static float fontScale = 1.0f;
static int fontAscent = 0;

void GLRenderer::initFont(float fontSize) {
    std::ifstream file(fontPath, std::ios::binary);
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, buffer.data(), 0)) {
        OutputDebugStringA("Failed to init font");
        return;
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

    fontScale = stbtt_ScaleForPixelHeight(&font, fontSize);
    fontAscent = static_cast<int>(ascent * fontScale);

    std::vector<unsigned char> atlas(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE);
    stbtt_BakeFontBitmap(buffer.data(), 0, fontSize, atlas.data(), FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, FONT_ASCII_START, FONT_ASCII_COUNT, cdata);

    glGenTextures(1, &fontTex);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void checkShader(GLuint s) {
    int success;
    char info[512];

    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(s, 512, nullptr, info);
        OutputDebugStringA(info);
    }
}

void checkProgram(GLuint p) {
    int success;
    char info[512];

    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(p, 512, nullptr, info);
        OutputDebugStringA(info);
    }
}

GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    checkShader(s);
    return s;
}

// Vertex shader
static const char* vs = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform vec2 uResolution;

void main() {
    vec2 zeroToOne = aPos / uResolution;
    vec2 zeroToTwo = zeroToOne * 2.0;
    vec2 clip = zeroToTwo - 1.0;

    gl_Position = vec4(clip.x, -clip.y, 0.0, 1.0);
    TexCoord = aTex;
}
)";

// Fragment shader
static const char* fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D uTexture;
uniform vec3 uColor;
uniform bool uUseTexture;

void main() {
    if (uUseTexture) {
        float a = texture(uTexture, TexCoord).r;
        FragColor = vec4(uColor, a);
    } else {
        FragColor = vec4(uColor, 1.0);
    }
}
)";

void GLRenderer::createShaders() {
    GLuint v = compile(GL_VERTEX_SHADER, vs);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs);

    shader.create();
    shader.attachShader(v);
    shader.attachShader(f);
    shader.link();
    checkProgram(shader);

    shader.use();
    shader.setInt("uTexture", 0);

    shader.deleteShader(v);
    shader.deleteShader(f);
}

void GLRenderer::initGLResources() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
}
#endif

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

#ifdef USE_OPENGL_11
    hglrc = wglCreateContext(hdc);
#elif USE_OPENGL_33
    HGLRC temp = wglCreateContext(hdc);
    wglMakeCurrent(hdc, temp);

    auto _wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    #define wglCreateContextAttribsARB _wglCreateContextAttribsARB

    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    hglrc = wglCreateContextAttribsARB(hdc, 0, attribs);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temp);
#endif

    wglMakeCurrent(hdc, hglrc);

#ifdef USE_OPENGL_33
    if (!gladLoadGL()) return false;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width, height);

    initGLResources();
    createShaders();
    initFont(48);
#elif USE_OPENGL_11
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    fontBase = glGenLists(256);
    HFONT font = CreateFontA(
        -20, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Segoe UI"
    );

    SelectObject(hdc, font);
    wglUseFontBitmapsA(hdc, 0, 256, fontBase);
    DeleteObject(font);
#endif
    return true;
}

void GLRenderer::clear(int r, int g, int b) {
    glClearColor(r / 255.0f, g / 255.0f,  b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLRenderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
#ifdef USE_OPENGL_33
    float verts[] = {
        (float)x, (float)y, 0.0f, 0.0f,
        (float)(x + w), (float)y, 0.0f, 0.0f,
        (float)(x + w), (float)(y + h), 0.0f, 0.0f,

        (float)x, (float)y, 0.0f, 0.0f,
        (float)(x + w), (float)(y + h), 0.0f, 0.0f,
        (float)x, (float)(y + h), 0.0f, 0.0f
    };

    shader.use();
    shader.setInt("uUseTexture", 0);
    shader.setVec2("uResolution", (float)width, (float)height);
    shader.setVec3("uColor", r / 255.0f, g / 255.0f, b / 255.0f);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLES, 0, 6);
#elif USE_OPENGL_11
    glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);

    glBegin(GL_QUADS);
        glVertex2f((float)x, (float)y);
        glVertex2f((float)(x + w), (float)y);
        glVertex2f((float)(x + w), (float)(y + h));
        glVertex2f((float)x, (float)(y + h));
    glEnd();
#endif
}

void GLRenderer::drawText(const char* text, float x, float y, float size, int r, int g, int b) {
#ifdef USE_OPENGL_33
    shader.use();

    shader.setInt("uUseTexture", 1);
    shader.setVec2("uResolution", (float)width, (float)height);
    shader.setVec3("uColor", r / 255.0f, g / 255.0f, b / 255.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    shader.setInt("uTexture", 0);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    float scale = size / 48.0f; // baked font size (48)
    float cursorX = x;
    float cursorY = y + fontAscent * (size / 48.0f); // baked font size (48)

    for (const char* p = text; *p; ++p) {
        if (*p < FONT_ASCII_START || *p >= FONT_ASCII_START + FONT_ASCII_COUNT) continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(cdata, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, *p - FONT_ASCII_START, &cursorX, &cursorY, &q, 1);

        float verts[] = {
            q.x0 * scale, q.y0 * scale, q.s0, q.t0,
            q.x1 * scale, q.y0 * scale, q.s1, q.t0,
            q.x1 * scale, q.y1 * scale, q.s1, q.t1,

            q.x0 * scale, q.y0 * scale, q.s0, q.t0,
            q.x1 * scale, q.y1 * scale, q.s1, q.t1,
            q.x0 * scale, q.y1 * scale, q.s0, q.t1
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
#elif USE_OPENGL_11
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);

    float ascent = size; // Text size (20.0f)

    glRasterPos2f(0, 0);
    glBitmap(0, 0, 0, 0, x, -(y + ascent), nullptr); // x, y is top left of text

    glListBase(fontBase);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#endif
}

void GLRenderer::present() {
    SwapBuffers(hdc);
}

void GLRenderer::resize(int w, int h) {
    width = w;
    height = h;

    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

#ifdef USE_OPENGL_11
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

GLRenderer::~GLRenderer() {    
#ifdef USE_OPENGL_11
    if (fontBase) glDeleteLists(fontBase, 256);
#endif

    wglMakeCurrent(NULL, NULL);
    if (hglrc) wglDeleteContext(hglrc);
    if (hdc) ReleaseDC(WindowFromDC(hdc), hdc);
}
#endif