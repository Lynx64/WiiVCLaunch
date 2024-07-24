#include "globals.hpp"
#include <coreinit/title.h>
#include <nn/cmpt/cmpt.h>
#include <sysapp/title.h>
#include <wups/function_patching.h>

static bool sAlreadyOverride = false;

DECL_FUNCTION(int32_t, CMPTAcctSetScreenType, CmptScreenType type)
{
    /*  we need to allow falling back to DRC only,
        so a "DRC only" argument is only overridden if this is the first time calling CMPTAcctSetScreenType(),
        after that we never override a "DRC only" argument */
    if (gForwarderDisplayOverride != DISPLAY_OPTION_CHOOSE &&
        (!sAlreadyOverride || type != CMPT_SCREEN_TYPE_DRC) &&
        OSGetTitleID() == _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_HEALTH_AND_SAFETY)) { //only do for homebrew
        sAlreadyOverride = true;
        type = (CmptScreenType) gForwarderDisplayOverride;
    }
    return real_CMPTAcctSetScreenType(type);
}

WUPS_MUST_REPLACE_FOR_PROCESS(CMPTAcctSetScreenType, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTAcctSetScreenType, WUPS_FP_TARGET_PROCESS_GAME);
