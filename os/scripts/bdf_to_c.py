# Example tff → bdf
# otf2bdf -p 16 -r 72 -o font.bdf font.ttf

import sys

def parse_bdf(filename: str):
    glyphs: dict[int, list[int]] = {}

    max_width = 0
    max_height = 0
    font_ascent = 0
    font_descent = 0

    with open(filename) as f:
        in_glyph = False
        in_bitmap = False

        encoding = None
        bitmap: list[int] = []
        glyph_height = 0
        offX = 0
        offY = 0

        for line in f:
            line = line.strip()

            if line.startswith('FONT_ASCENT'):
                font_ascent = int(line.split()[1])
            elif line.startswith('FONT_DESCENT'):
                font_descent = int(line.split()[1])
            elif line.startswith('STARTCHAR'):
                in_glyph = True
                encoding = None
                bitmap = []
                glyph_height = 0
                offX = 0
                offY = 0
                in_bitmap = False
                continue

            if not in_glyph:
                continue

            if line.startswith('ENCODING'):
                encoding = int(line.split()[1])

            elif line.startswith('BBX'):
                parts = line.split()
                w = int(parts[1])
                h = int(parts[2])
                offX = int(parts[3])
                offY = int(parts[4])

                max_width = max(max_width, w)
                max_height = max(max_height, h)

                glyph_height = h

            elif line == 'BITMAP':
                in_bitmap = True
                continue

            elif line == 'ENDCHAR':
                if encoding is not None and 0 <= encoding < 256:
                    top_pad = font_ascent - glyph_height - offY
                    if top_pad < 0: top_pad = 0
                    
                    bitmap = [0] * top_pad + bitmap
                    
                    total_h = font_ascent + font_descent
                    max_height = max(max_height, total_h)
                    
                    bottom_pad = max_height - len(bitmap)
                    if bottom_pad > 0:
                        bitmap = bitmap + [0] * bottom_pad
                    elif len(bitmap) > max_height:
                        bitmap = bitmap[:max_height]

                    glyphs[encoding] = bitmap

                in_glyph = False
                in_bitmap = False
                continue

            elif in_bitmap:
                hex_str = line.ljust(4, '0')[:4]
                try:
                    row = int(hex_str, 16)
                except ValueError:
                    row = 0
                
                if offX > 0:
                    row >>= offX
                elif offX < 0:
                    row <<= -offX
                
                bitmap.append(row & 0xFFFF)

    return glyphs, max_width, max_height

def write_c_header(width: int, height: int, out_h: str):
    with open(out_h, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include <lib/stdint.h>\n\n')
        
        f.write(f'#define FONT_WIDTH {width}\n')
        f.write(f'#define FONT_HEIGHT {height}\n\n')

        f.write(f'extern const uint16_t font[256][{height}];\n')

def write_c_source(glyphs: dict[int, list[int]], height: int, out_h_name: str, out_c: str):
    with open(out_c, 'w') as f:
        f.write(f'#include "{out_h_name}"\n\n')
        f.write(f'const uint16_t font[256][{height}] = {{\n')

        for i in range(256):
            glyph = glyphs.get(i, [])

            if len(glyph) > height:
                glyph = glyph[:height]
            elif len(glyph) < height:
                glyph = glyph + [0] * (height - len(glyph))

            f.write('    { ')
            f.write(', '.join(f'0x{row:04x}' for row in glyph))
            f.write('},\n')

        f.write('};\n')

def main():
    if len(sys.argv) != 4:
        print('Usage: python bdf_to_c.py font.bdf font.h font.c')
        return
    
    if not sys.argv[1].endswith('.bdf'):
        print('Only BDF font files are supported')
        return
    
    import os
    out_h_name = os.path.basename(sys.argv[2])
    
    glyphs, w, h = parse_bdf(sys.argv[1])
    write_c_header(w, h, sys.argv[2])
    write_c_source(glyphs, h, out_h_name, sys.argv[3])

    print(f'Font converted: {w}x{h}, {len(glyphs)} glyphs')

if __name__ == '__main__':
    main()