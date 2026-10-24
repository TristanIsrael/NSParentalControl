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

#include "screen_timeout.h"
#include "font.h"
#include "logger.h"
#include "helpers.h"
#include "utils.h"
#include <cstring>

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::database;
using namespace ams;

namespace alefbet::pctrl::gui {    

    namespace {

        /* Screen definitions. */
        constexpr u32 PctrlScreenWidth = 1280;
        constexpr u32 PctrlScreenHeight = 720;
        constexpr u32 PctrlScreenBpp = 2;
        constexpr u32 PctrlLayerZ = 100;

        constexpr u32 PctrlScreenWidthAlignedBytes = util::AlignUp(PctrlScreenWidth * PctrlScreenBpp, 64);
        constexpr u32 PctrlScreenWidthAligned = PctrlScreenWidthAlignedBytes / PctrlScreenBpp;        

        /* There should only be a single (1280*768) framebuffer. */
        constexpr size_t FrameBufferRequiredSizeBytes       = PctrlScreenWidthAlignedBytes * util::AlignUp(PctrlScreenHeight, 128);
        constexpr size_t FrameBufferRequiredSizePageAligned = util::AlignUp(FrameBufferRequiredSizeBytes, os::MemoryPageSize);
        constexpr size_t FrameBufferRequiredSizeHeapAligned = util::AlignUp(FrameBufferRequiredSizeBytes, os::MemoryHeapUnitSize);

        constinit u8 *g_framebuffer_pointer = nullptr;      
        
        bool sharedFontInitialized = false;
    }      
    
    namespace {
        #include "inc/logo.inc"
    }

    /* Pixel calculation helper. */
    constexpr u32 GetPixelOffset(u32 x, u32 y) {
        u32 tmp_pos = ((y & 127) / 16) + (x/32*8) + ((y/16/8)*(((PctrlScreenWidthAligned/2)/16*8)));
        tmp_pos *= 16*16 * 4;

        tmp_pos += ((y%16)/8)*512 + ((x%32)/16)*256 + ((y%8)/2)*64 + ((x%16)/8)*32 + (y%2)*16 + (x%8)*2; //This line is a modified version of code from the Tegra X1 datasheet.

        return tmp_pos / 2;
    }

    void ScreenTimeout::InitializeFrameBufferPointer() {
        void* mem = aligned_alloc(0x1000, FrameBufferRequiredSizePageAligned);
        if(mem == nullptr) {
            logError("[Screen timeout] Could not allocate %i bytes of memory\n", FrameBufferRequiredSizePageAligned);
        }

        g_framebuffer_pointer = reinterpret_cast<u8*>(mem);
    }

    /* Task implementations. */
    ::Result ScreenTimeout::SetupDisplayInternal() {
        logDebug("[Screen] SetupDisplayInternal\n");

        ViDisplay temp_display;

        /* Try to open the display. */
        ::Result res = viOpenDisplay("Internal", std::addressof(temp_display));
        if(R_FAILED(res)) {
            // Do not overreact
            logError("[Screen] Could not open internal display\n");
            // but fail...
            return res;
        }        
        
        logDebug("[Screen] Turn the screen on.\n");
        viSetDisplayPowerState(std::addressof(temp_display), ViPowerState_On);        

        /* Set alpha to 1.0f. */
        res = viSetDisplayAlpha(std::addressof(temp_display), 1.0f);
        if(R_FAILED(res)) {
            logError("[Screen] Could not set internal display alpha\n");
        }

        viCloseDisplay(std::addressof(temp_display));

        return 0;
    }

    ::Result ScreenTimeout::SetupDisplayExternal() {
        logDebug("[Screen] SetupDisplayExternal\n");

        ViDisplay temp_display;
        /* Try to open the display. */
        ::Result res = viOpenDisplay("External", std::addressof(temp_display));
        if(R_FAILED(res)) {
            // Do not overreact
            logError("[Screen] Could not open external display\n");
            // but fail...
            return res;
        }

        /* Set alpha to 1.0f. */
        res = viSetDisplayAlpha(std::addressof(temp_display), 1.0f);
        if(R_FAILED(res)) {
            logError("[Screen] Could not set external display alpha\n");
        }

        viCloseDisplay(std::addressof(temp_display));

        return 0;
    }

    ::Result ScreenTimeout::PrepareScreenForDrawing() {    
        logDebug("[Screen] PrepareScreenForDrawing\n");

        /* Connect to vi. */
        ::Result res = viInitialize(ViServiceType_Manager);
        if(R_FAILED(res)) {
            logError("[Screen] Could not initialise VI service\n");
            return res; //Fail
        }

        /* Setup the two displays. */
        // Ignore errors (they are logged)
        SetupDisplayInternal();
        SetupDisplayExternal();

        /* Open the default display. */
        TRY_AND_RETURN_2(
            viOpenDefaultDisplay(std::addressof(m_display)), 
            "[Screen] Could not open default display. Aborting.\n"
        )

        /* Reset the display magnification to its default value. */
        s32 display_width, display_height;
        TRY_AND_RETURN_2(
            viGetDisplayLogicalResolution(std::addressof(m_display), std::addressof(display_width), std::addressof(display_height)),
            "[Screen] Could not get display logical resolution.\n"
        )
 
        /* Magnify */
        TRY_AND_RETURN_2(
            viSetDisplayMagnification(std::addressof(m_display), 0, 0, display_width, display_height),
            "[Screen] Could not reset display magnification.\n"
        )

        /* Create layer to draw to. */
        TRY_AND_RETURN_2(
            viCreateLayer(std::addressof(m_display), std::addressof(m_layer)),
            "[Screen] Could not create layer.\n"
        )

        logDebug("[Screen] Layer address=%p\n", (void*)std::addressof(m_layer));

        /* Setup the layer. */
        {
            /* Display a layer of 1280 x 720 at 1.5x magnification */
            /* NOTE: N uses 2 (770x400) RGBA4444 buffers (tiled buffer + linear). */
            /* We use a single 1280x720 tiled RGB565 buffer. */
            constexpr s32 RawWidth = PctrlScreenWidth;
            constexpr s32 RawHeight = PctrlScreenHeight;
            constexpr s32 LayerWidth = ((RawWidth) * 3) / 2;
            constexpr s32 LayerHeight = ((RawHeight) * 3) / 2;

            const float layer_x = static_cast<float>((display_width - LayerWidth) / 2);
            const float layer_y = static_cast<float>((display_height - LayerHeight) / 2);

            TRY_AND_RETURN_2(
                viSetLayerSize(std::addressof(m_layer), LayerWidth, LayerHeight),
                "[Screen] Could not set layer size.\n"
            )

            /* Set the layer's Z at display maximum, to be above everything else .*/
            TRY_AND_RETURN_2(
                viSetLayerZ(std::addressof(m_layer), PctrlLayerZ),
                "[Screen] Could not set layer z position.\n"
            )

            /* Center the layer in the screen. */
            TRY_AND_RETURN_2(
                viSetLayerPosition(std::addressof(m_layer), layer_x, layer_y),
                "[Screen] Could not set layer position.\n"
            )

            /* Create framebuffer. */
            TRY_AND_RETURN_2(
                nwindowCreateFromLayer(std::addressof(m_win), std::addressof(m_layer)),
                "[Screen] Could not create nwindow from layer.\n"
            )

            TRY_AND_RETURN_2(
                InitializeNativeWindow(),
                "[Screen] Could not initialize native window.\n"
            )
        }        

        return 0;
    }

    void ScreenTimeout::PreRenderFrameBuffer() {   
        logDebug("[Screen] PreRenderFrameBuffer\n");

        /* Allocate a frame buffer. */
        InitializeFrameBufferPointer();
        if(g_framebuffer_pointer == nullptr) {
            logError("[Screen] The framebuffer pointer is null. Aborting.\n");
            return;
        }  
        
        PreRenderContents();
    }

    ::Result ScreenTimeout::PreRenderContents() {
        logDebug("[Screen] PreRenderContents\n");

        if(!sharedFontInitialized) {
            /* Load shared font. */
            ::Result res = alefbet::pctrl::font::InitializeSharedFont();
            if(R_FAILED(res)) {
                logError("[Screen] Could not initialize shared font\n");
                return 1;
            } else {
                logError("[Screen] Shared font loaded\n");
                sharedFontInitialized = true;
            }
        }

        /* Pre-render the image into the static framebuffer. */
        u16 *tiled_buf = reinterpret_cast<u16 *>(g_framebuffer_pointer);

        /* Let the font manager know about our framebuffer. */
        font::ConfigureFontFramebuffer(tiled_buf, GetPixelOffset);
        font::SetFontColor(0x000F);

        /* Draw a background. */
        for (size_t i = 0; i < FrameBufferRequiredSizeBytes / sizeof(*tiled_buf); i++) {
            tiled_buf[i] = pctrl_logo[0];
        }
        
        u32 logo_x = (PctrlScreenWidth - LogoWidth) / 2;
        u32 logo_y = (PctrlScreenHeight - LogoHeight) / 2;

        /* Draw the logo in the center of the screen */
        for (size_t y = 0; y < LogoHeight; y++) {
            for (size_t x = 0; x < LogoWidth; x++) {
                tiled_buf[GetPixelOffset(logo_x + x, logo_y + y)] = pctrl_logo[y * LogoWidth + x];
            }
        }

        /* Show message */
        font::SetPosition(336, 318);
        font::SetFontSize(62.0f);
        font::Print("Your time is up!");
        font::SetPosition(473, 402);
        font::SetFontSize(25.0f);
        font::Print("Press Vol+ to reboot");
        
        return 0;
    }

    ::Result ScreenTimeout::InitializeNativeWindow() {
        logDebug("[Screen] InitializeNativeWindow\n");

        //* Setup nv driver. */
        TRY_AND_RETURN_2(
            nvInitialize(),
            "[Screen] Could not initialize NV service\n"
        )

        TRY_AND_RETURN_2(
            nvMapInit(),
            "[Screen] Could not init Map\n"
        )

        TRY_AND_RETURN_2(
            nvFenceInit(),
            "[Screen] Could not init fence\n"
        )

        /* Create nvmap. */
        TRY_AND_RETURN_2(
            nvMapCreate(std::addressof(m_map), g_framebuffer_pointer, FrameBufferRequiredSizeBytes, 0x20000, NvKind_Pitch, true),
            "[Screen] Could not create Map\n"
        )

        /* Setup graphics buffer. */
        {
            NvGraphicBuffer grbuf               = {};
            grbuf.header.num_ints               = (sizeof(NvGraphicBuffer) - sizeof(NativeHandle)) / 4;
            grbuf.unk0                          = -1;
            grbuf.magic                         = 0xDAFFCAFF;
            grbuf.pid                           = 42;
            grbuf.usage                         = GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE;
            grbuf.format                        = PIXEL_FORMAT_RGB_565;
            grbuf.ext_format                    = PIXEL_FORMAT_RGB_565;
            grbuf.num_planes                    = 1;
            grbuf.planes[0].width               = PctrlScreenWidth;
            grbuf.planes[0].height              = PctrlScreenHeight;
            grbuf.planes[0].color_format        = NvColorFormat_R5G6B5;
            grbuf.planes[0].layout              = NvLayout_BlockLinear;
            grbuf.planes[0].kind                = NvKind_Generic_16BX2;
            grbuf.planes[0].block_height_log2   = 4;
            grbuf.nvmap_id                      = nvMapGetId(std::addressof(m_map));
            grbuf.stride                        = PctrlScreenWidthAligned;
            grbuf.total_size                    = FrameBufferRequiredSizeBytes;
            grbuf.planes[0].pitch               = PctrlScreenWidthAlignedBytes;
            grbuf.planes[0].size                = FrameBufferRequiredSizeBytes;
            grbuf.planes[0].offset              = 0;            

            TRY_AND_RETURN_2(
                nwindowConfigureBuffer(std::addressof(m_win), 0, std::addressof(grbuf)),
                "[Screen] Could not configure nwindow buffer\n"
            )
        }

        return 0;        
    }

    void ScreenTimeout::DisplayPreRenderedFrame() {
        logDebug("[Screen] DisplayPreRenderedFrame\n");

        s32 slot;

        logDebug("[Screen] Before nwindowDequeueBuffer\n");
        ::Result res = nwindowDequeueBuffer(std::addressof(m_win), std::addressof(slot), nullptr);
        if(R_FAILED(res)) {
            logError("[Screen] Could not dequeue buffer for nwindow: %i:%i\n", R_MODULE(res), R_DESCRIPTION(res));
            return;
        }
        
        armDCacheFlush(g_framebuffer_pointer, FrameBufferRequiredSizeBytes);        
        res = nwindowQueueBuffer(std::addressof(m_win), m_win.cur_slot, NULL);

        if(R_FAILED(res)) {
            logError("[Screen timeout] Coulr not dequeue window buffer: %i.%i\n", R_MODULE(res), R_DESCRIPTION(res));
        }
    }

    ::Result ScreenTimeout::ShowScreenTimeout() {
        logDebug("[Screen] Show timeout screen\n");       

        /* Pre-render the framebuffer. */
        this->PreRenderFrameBuffer();

        /* Prepare screen for drawing. */
        ::Result res = this->PrepareScreenForDrawing();
        if(R_FAILED(res)) {
            logError("[Screen] PrepareScreenForDrawing failed\n");
            return res;
        }

        /* Display the pre-rendered frame. */
        this->DisplayPreRenderedFrame();       

        return 0;
    }

}