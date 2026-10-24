/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "font.h"
#include "logger.h"
#include <memory>
#include <cstdarg>
#include <cstring>
//#include "utils.h"

//using namespace ams;
using namespace alefbet::pctrl::logger;

/*namespace alefbet::pctrl::font {

    constinit lmem::HeapHandle g_font_heap_handle;

    void SetHeapMemory(void *memory, size_t memory_size) {
        g_font_heap_handle = lmem::CreateExpHeap(memory, memory_size, lmem::CreateOption_None);
    }

    void *AllocateForFont(size_t size) {
        void *heap = lmem::AllocateFromExpHeap(g_font_heap_handle, size);

        return heap;
    }

    void DeallocateForFont(void *p) {
        if (p != nullptr) {
            return lmem::FreeToExpHeap(g_font_heap_handle, p);
        }
    }
}*/

/*#define STBTT_assert(x)    AMS_ASSERT(x)
#define STBTT_malloc(x,u)  ((void)(u),alefbet::pctrl::font::AllocateForFont(x))
#define STBTT_free(x,u)    ((void)(u),alefbet::pctrl::font::DeallocateForFont(x))

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#undef  STBTT_STATIC
#undef  STB_TRUETYPE_IMPLEMENTATION

#undef  STBTT_malloc
#undef  STBTT_free
#undef  STBTT_assert*/

//#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/* Define color conversion macros. */
#define RGB565_GET_R8(c) ((((c >> 11) & 0x1F) << 3) | ((c >> 13) & 7))
#define RGB565_GET_G8(c) ((((c >> 5) & 0x3F) << 2) | ((c >> 9) & 3))
#define RGB565_GET_B8(c) ((((c >> 0) & 0x1F) << 3) | ((c >> 2) & 7))

namespace alefbet::pctrl::font {

    namespace {

        /* Font state globals. */
        u16 *g_frame_buffer = nullptr;
        u32 (*g_unswizzle_func)(u32, u32) = nullptr;
        u16 g_font_color = 0xFFFF; /* White. */
        float g_font_line_pixels = 16.0f;
        float g_font_size = 16.0f;
        u32 g_line_x = 0, g_cur_x = 0, g_cur_y = 0;

        u32 g_mono_adv = 0;

        PlFontData g_font;

        stbtt_fontinfo g_stb_font;

        /* Helpers. */
        /*! @brief Blends two RGB565 colors with the given alpha value.
         * @param color The foreground color.
         * @param bg The background color.
         * @param alpha The alpha value (0-255).
         * @return The blended RGB565 color. */
        u16 Blend(u16 color, u16 bg, u8 alpha) {
            const u32 c_r = RGB565_GET_R8(color);
            const u32 c_g = RGB565_GET_G8(color);
            const u32 c_b = RGB565_GET_B8(color);
            const u32 b_r = RGB565_GET_R8(bg);
            const u32 b_g = RGB565_GET_G8(bg);
            const u32 b_b = RGB565_GET_B8(bg);

            const u32 r = ((alpha * c_r) + ((0xFF - alpha) * b_r)) / 0xFF;
            const u32 g = ((alpha * c_g) + ((0xFF - alpha) * b_g)) / 0xFF;
            const u32 b = ((alpha * c_b) + ((0xFF - alpha) * b_b)) / 0xFF;

            return RGB888_TO_RGB565(r, g, b);
        }

        /*! @brief Draws a single codepoint glyph at the given position on the framebuffer.
         * @param codepoint The Unicode codepoint to draw.
         * @param x The X coordinate.
         * @param y The Y coordinate. */
        void DrawCodePoint(u32 codepoint, u32 x, u32 y) {
            int width = 0, height = 0;
            u8* imageptr = stbtt_GetCodepointBitmap(std::addressof(g_stb_font), g_font_size, g_font_size, codepoint, std::addressof(width), std::addressof(height), 0, 0);
            //ON_SCOPE_EXIT { DeallocateForFont(imageptr); };

            for (int tmpy = 0; tmpy < height; tmpy++) {
                for (int tmpx = 0; tmpx < width; tmpx++) {
                    /* Implement very simple blending, as the bitmap value is an alpha value. */
                    u16 *ptr = g_frame_buffer + g_unswizzle_func(x + tmpx, y + tmpy);
                    *ptr = Blend(g_font_color, *ptr, imageptr[width * tmpy + tmpx]);
                }
            }
        }

        /*! @brief Draws a string onto the framebuffer at the current position.
         * @param str The null-terminated string to draw.
         * @param add_line Whether to advance to the next line after drawing.
         * @param mono Whether to use monospace rendering. */
        void DrawString(const char *str, bool add_line, bool mono = false) {
            const size_t len = strlen(str);

            u32 cur_x = g_cur_x, cur_y = g_cur_y;

            u32 prev_char = 0;
            for (u32 i = 0; i < len; ) {
                u32 cur_char;
                ssize_t unit_count = decode_utf8(std::addressof(cur_char), reinterpret_cast<const u8 *>(str + i));
                if (unit_count <= 0) break;

                if (!g_mono_adv && i > 0) {
                    cur_x += g_font_size * stbtt_GetCodepointKernAdvance(std::addressof(g_stb_font), prev_char, cur_char);
                }

                i += unit_count;

                if (cur_char == '\n') {
                    cur_x = g_line_x;
                    cur_y += g_font_line_pixels;
                    continue;
                }

                int adv_width, left_side_bearing;
                stbtt_GetCodepointHMetrics(std::addressof(g_stb_font), cur_char, std::addressof(adv_width), std::addressof(left_side_bearing));
                const u32 cur_width = static_cast<u32>(adv_width) * g_font_size;

                int x0, y0, x1, y1;
                stbtt_GetCodepointBitmapBoxSubpixel(std::addressof(g_stb_font), cur_char, g_font_size, g_font_size, 0, 0, std::addressof(x0), std::addressof(y0), std::addressof(x1), std::addressof(y1));

                DrawCodePoint(cur_char, cur_x + x0 + ((mono && g_mono_adv > cur_width) ? ((g_mono_adv - cur_width) / 2) : 0), cur_y + y0);

                cur_x += (mono ? g_mono_adv : cur_width);

                prev_char = cur_char;
            }

            if (add_line) {
                /* Advance to next line. */
                g_cur_x = g_line_x;
                g_cur_y = cur_y + g_font_line_pixels;
            } else {
                g_cur_x = cur_x;
                g_cur_y = cur_y;
            }
        }

    }

            /*! @brief Prints a string and advances to the next line.
         * @param str The null-terminated string to print. */
        void PrintLine(const char *str) {
        return DrawString(str, true);
    }

    /*void PrintFormatLine(const char *format, ...) {
        char char_buf[0x400];

        std::va_list va_arg;
        va_start(va_arg, format);
        util::VSNPrintf(char_buf, sizeof(char_buf), format, va_arg);
        va_end(va_arg);

        PrintLine(char_buf);
    }*/

            /*! @brief Prints a string without advancing to the next line.
         * @param str The null-terminated string to print. */
        void Print(const char *str) {
        return DrawString(str, false);
    }

    /*void PrintFormat(const char *format, ...) {
        char char_buf[0x400];

        std::va_list va_arg;
        va_start(va_arg, format);
        util::VSNPrintf(char_buf, sizeof(char_buf), format, va_arg);
        va_end(va_arg);

        Print(char_buf);
    }

    void PrintMonospaceU64(u64 x) {
        char char_buf[0x400];
        util::SNPrintf(char_buf, sizeof(char_buf), "%016lX", x);

        DrawString(char_buf, false, true);
    }

    void PrintMonospaceU32(u32 x) {
        char char_buf[0x400];
        util::SNPrintf(char_buf, sizeof(char_buf), "%08X", x);

        DrawString(char_buf, false, true);
    }

    void PrintMonospaceBlank(u32 width) {
        char char_buf[0x400] = {0};
        std::memset(char_buf, ' ', std::min(size_t(width), sizeof(char_buf)));

        DrawString(char_buf, false, true);
    }*/

            /*! @brief Sets the current font color (RGB565 format).
         * @param color The RGB565 color value. */
        void SetFontColor(u16 color) {
        g_font_color = color;
    }

            /*! @brief Sets the current drawing position.
         * @param x The X coordinate.
         * @param y The Y coordinate. */
        void SetPosition(u32 x, u32 y) {
        g_line_x = x;
        g_cur_x = x;
        g_cur_y = y;
    }

            /*! @brief Returns the current X coordinate.
         * @return The current X coordinate. */
        u32 GetX() {
        return g_cur_x;
    }

            /*! @brief Returns the current Y coordinate.
         * @return The current Y coordinate. */
        u32 GetY() {
        return g_cur_y;
    }

            /*! @brief Sets the font size and recalculates metrics.
         * @param fsz The font size in pixels. */
        void SetFontSize(float fsz) {
        g_font_size = stbtt_ScaleForPixelHeight(std::addressof(g_stb_font), fsz * 1.375);

        int ascent;
        stbtt_GetFontVMetrics(std::addressof(g_stb_font), std::addressof(ascent),0,0);
        g_font_line_pixels = ascent * g_font_size * 1.125;

        int adv_width, left_side_bearing;
        stbtt_GetCodepointHMetrics(std::addressof(g_stb_font), 'A', std::addressof(adv_width), std::addressof(left_side_bearing));

        g_mono_adv = adv_width * g_font_size;
    }

            /*! @brief Adds spacing lines by advancing the Y coordinate.
         * @param num_lines Number of lines to add as spacing. */
        void AddSpacingLines(float num_lines) {
        g_cur_x = g_line_x;
        g_cur_y += static_cast<u32>(g_font_line_pixels * num_lines);
    }

            /*! @brief Configures the framebuffer and unswizzle function for font rendering.
         * @param fb Pointer to the framebuffer (u16 array).
         * @param unswizzle_func Function to convert linear coordinates to framebuffer offset. */
        void ConfigureFontFramebuffer(u16 *fb, u32 (*unswizzle_func)(u32, u32)) {        
        g_frame_buffer = fb;
        g_unswizzle_func = unswizzle_func;
    }

            /*! @brief Initializes the shared font using the PL (PlayLog) service and stb_truetype.
         * @return Result code, 0 on success. */
        Result InitializeSharedFont() {
        ::Result rc = plGetSharedFontByType(std::addressof(g_font), PlSharedFontType_Standard);
        if(rc != 0) {
            logError("[Font] font initialization failed\n");
        }

        u8 *font_buffer = reinterpret_cast<u8 *>(g_font.address);
        stbtt_InitFont(std::addressof(g_stb_font), font_buffer, stbtt_GetFontOffsetForIndex(font_buffer, 0));

        SetFontSize(16.0f);
        return 0;
    }

}