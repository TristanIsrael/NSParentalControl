#pragma once
#include "types.h"

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

        private:
            ViDisplay m_display;
            ViLayer m_layer;
            NWindow m_win;
            NvMap m_map;
            //u8* g_nv_transfer_memory = nullptr;
            //u64 nv_transfer_memory_size = 0;
    };
    
}