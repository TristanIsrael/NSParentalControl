#pragma once
#include "types.h"

#define RGB888_TO_RGB565(r, g, b) ((((r >> 3) << 11) & 0xF800) | (((g >> 2) << 5) & 0x7E0) | ((b >> 3) & 0x1F))

namespace alefbet::pctrl::font {

    Result InitializeSharedFont();
    void ConfigureFontFramebuffer(u16 *fb, u32 (*unswizzle_func)(u32, u32));
    void SetHeapMemory(void *memory, size_t memory_size);

    void SetFontColor(u16 color);
    void SetPosition(u32 x, u32 y);
    u32 GetX();
    u32 GetY();
    void SetFontSize(float fsz);
    void AddSpacingLines(float num_lines);
    void PrintLine(const char *str);
    void PrintFormatLine(const char *format, ...);
    void Print(const char *str);
    void PrintFormat(const char *format, ...);
    void PrintMonospaceU64(u64 x);
    void PrintMonospaceU32(u32 x);
    void PrintMonospaceBlank(u32 width);

}