#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_truetype.h"
#include "stb_image_write.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>

static const char* fontPath = "../src/fonts/segoe-ui.ttf";

static const int ATLAS_WIDTH  = 256;
static const int ATLAS_HEIGHT = 128;

static const int FIRST_CHAR = 32;
static const int CHAR_COUNT = 95;

struct Glyph {
    float x0, y0, x1, y1;   // UV coords
    float xoff, yoff;       // offset from cursor
    float xadvance;         // cursor advance
};

int main() {
    // ------------------------------------------------------------
    // Load TTF file
    // ------------------------------------------------------------
    std::ifstream file(fontPath, std::ios::binary);
    if (!file) {
        std::cout << "Failed to open font file\n";
        return 1;
    }

    std::vector<unsigned char> ttf(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, ttf.data(), 0)) {
        std::cout << "Failed to init font\n";
        return 1;
    }

    // ------------------------------------------------------------
    // Bake font
    // ------------------------------------------------------------
    float fontSize = 32.0f;

    std::vector<unsigned char> bitmap(ATLAS_WIDTH * ATLAS_HEIGHT);
    stbtt_bakedchar baked[CHAR_COUNT];

    int result = stbtt_BakeFontBitmap(
        ttf.data(), 0,
        fontSize,
        bitmap.data(),
        ATLAS_WIDTH,
        ATLAS_HEIGHT,
        FIRST_CHAR,
        CHAR_COUNT,
        baked
    );

    if (result <= 0) {
        std::cout << "Atlas too small!\n";
        return 1;
    }

    std::cout << "Glyphs packed: " << result << "\n";

    // ------------------------------------------------------------
    // Save full atlas (IMPORTANT: do NOT crop UV space)
    // ------------------------------------------------------------
    stbi_write_png(
        "font_atlas.png",
        ATLAS_WIDTH,
        ATLAS_HEIGHT,
        1,
        bitmap.data(),
        ATLAS_WIDTH
    );

    // ------------------------------------------------------------
    // Convert baked data → stable GPU glyph metrics
    // ------------------------------------------------------------
    std::ofstream out("font_metrics.bin", std::ios::binary);
    if (!out) {
        std::cout << "Failed to write metrics file\n";
        return 1;
    }

    for (int i = 0; i < CHAR_COUNT; i++) {
        const stbtt_bakedchar& b = baked[i];

        Glyph g;

        // ✅ FIX: ALWAYS normalize using FULL atlas size
        // (this is the correct OpenGL UV mapping)
        g.x0 = b.x0 / (float)ATLAS_WIDTH;
        g.y0 = b.y0 / (float)ATLAS_HEIGHT;
        g.x1 = b.x1 / (float)ATLAS_WIDTH;
        g.y1 = b.y1 / (float)ATLAS_HEIGHT;

        g.xoff = b.xoff;
        g.yoff = b.yoff;
        g.xadvance = b.xadvance;

        out.write(reinterpret_cast<const char*>(&g), sizeof(Glyph));
    }

    std::cout << "Done. Font assets generated.\n";
    return 0;
}