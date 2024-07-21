#include "globals.hpp"
#include "logger.h"
#include <notifications/notifications.h>

void applyNotificationThemeSetting()
{
    // NOTIFICATION_THEME_DARK
    NMColor notifTextColour       = {255, 255, 255, 255};
    NMColor notifBackgroundColour = {100, 100, 100, 255};

    if (gNotificationTheme == NOTIFICATION_THEME_LIGHT) {
        notifTextColour       = {0, 0, 0, 255};
        notifBackgroundColour = {250, 250, 250, 255};
    }

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                       NOTIFICATION_MODULE_DEFAULT_OPTION_TEXT_COLOR,
                                       notifTextColour);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                       NOTIFICATION_MODULE_DEFAULT_OPTION_BACKGROUND_COLOR,
                                       notifBackgroundColour);
}

void initNotifications()
{
    NotificationModuleStatus notifStatus = NotificationModule_InitLibrary();
    if (notifStatus != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("NotificationModule_InitLibrary returned %s (%d)",
                                NotificationModule_GetStatusStr(notifStatus),
                                notifStatus);
        return;
    }

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                       NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT,
                                       3.0f);
    applyNotificationThemeSetting();
}
