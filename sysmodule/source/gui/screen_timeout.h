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

#pragma once
#include <switch.h>

namespace alefbet::pctrl::gui {

    #define TRY_AND_RETURN_2(res_expr, message)                                 \
    {                                                                           \
        const auto _tmp_r_try_rc = (res_expr);                                  \
        if (R_FAILED(_tmp_r_try_rc)) {                                          \
            alefbet::pctrl::logger::logError(message);                          \
            alefbet::pctrl::logger::logError("Error %i:%i\n", R_MODULE(_tmp_r_try_rc), R_DESCRIPTION(_tmp_r_try_rc));   \
            return 1;                                                           \
        }                                                                       \
    }                                                                           \
    
    class ScreenTimeout {            
        private:
            void     InitializeFrameBufferPointer();
            ::Result SetupDisplayInternal();
            ::Result SetupDisplayExternal();
            ::Result PrepareScreenForDrawing();
            void     PreRenderFrameBuffer();
            ::Result PreRenderContents();
            ::Result InitializeNativeWindow();
            void     DisplayPreRenderedFrame();   
            //bool     WaitForVolumeButton();       

        public:            
            //void setTransferMemory(u8*, u64 size);
            ::Result ShowScreenTimeout();
            void Hide();

        private:
            ViDisplay m_display;
            ViLayer m_layer;
            NWindow m_win;
            NvMap m_map;
            //u8* g_nv_transfer_memory = nullptr;
            //u64 nv_transfer_memory_size = 0;
            bool m_display_prepared = false;
    };
    
}