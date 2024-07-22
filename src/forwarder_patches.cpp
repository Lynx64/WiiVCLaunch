#include "globals.hpp"
#include <coreinit/title.h>
#include <nn/cmpt/cmpt.h>
#include <sysapp/title.h>
#include <wups/function_patching.h>

DECL_FUNCTION(int32_t, CMPTAcctSetScreenType, CmptScreenType type)
{
    if (gForwarderDisplayOverride != DISPLAY_OPTION_CHOOSE &&
        OSGetTitleID() == _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_HEALTH_AND_SAFETY)) //only do for homebrew

        type = (CmptScreenType) gForwarderDisplayOverride;
    return real_CMPTAcctSetScreenType(type);
}

WUPS_MUST_REPLACE_FOR_PROCESS(CMPTAcctSetScreenType, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTAcctSetScreenType, WUPS_FP_TARGET_PROCESS_GAME);
