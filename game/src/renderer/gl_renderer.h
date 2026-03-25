#pragma once
#ifdef USE_OPENGL
#include "../core/renderer.h"
#ifdef USE_OPENGL_33
#include "./vendor/glad.h"
#elif USE_OPENGL_11
#include <GL/gl.h>
#endif

#ifdef USE_OPENGL_33
class GLShader {
public:
    GLuint id;

    // GLShader(const GLShader&) = delete;
    // GLShader& operator=(const GLShader&) = delete;

    void create() {
        if (id != 0) {
            glDeleteProgram(id);
            id = 0;
        }
        id = glCreateProgram();
    }

    // I'm ready to get flamed for this
    operator GLuint() const noexcept {
        return id;
    }

    void use() const {
        glUseProgram(id);
    }

    void setVec2(const char* name, float x, float y) const {
        glUniform2f(glGetUniformLocation(id, name), x, y);
    }

    void setVec3(const char* name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(id, name), x, y, z);
    }

    void setInt(const char* name, int v) const {
        glUniform1i(glGetUniformLocation(id, name), v);
    }

    void attachShader(GLuint cShader) const {
        glAttachShader(id, cShader);
    }

    void deleteShader(GLuint cShader) const {
        glDeleteShader(cShader);
    }

    void link() const {
        glLinkProgram(id);
    }
};
#endif

class GLRenderer : public IRenderer {
public:
    bool init(HWND hwnd, int width, int height) override;

    void clear(int r, int g, int b) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void drawText(const char* text, float x, float y, float size, int r, int g, int b) override;
    void present() override;

    void resize(int w, int h) override;
    int getWidth() const override { return width; }
    int getHeight() const override { return height; }

    ~GLRenderer();

private:
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;
    
#ifdef USE_OPENGL_33
    void initFont(float fontSize);
    void initGLResources();
    void createShaders();

    GLuint vao = 0;
    GLuint vbo = 0;
    GLShader shader;
#elif USE_OPENGL_11
    GLuint fontBase = 0;
#endif

    int width = 0;
    int height = 0;
};
#endif