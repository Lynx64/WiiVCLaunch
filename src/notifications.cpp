#include "notifications.h"
#include "globals.hpp"
#include "logger.h"

#include <coreinit/time.h>

#include <notifications/notifications.h>

static OSTime sSensorBarCooldown = 0;

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

    applyNotificationThemeSetting();
}

void showSensorBarNotification(const char *text)
{
    if (!gSensorBarNotifEnabled) {
        return;
    }

    // Don't create multiple sensor bar notifications at once.
    if (sSensorBarCooldown > OSGetSystemTime()) {
        return;
    }
    sSensorBarCooldown = OSGetSystemTime() + OSSecondsToTicks(gSensorBarNotifDurationSecs);

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                       NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT,
                                       gSensorBarNotifDurationSecs);

    NotificationModuleStatus result = NotificationModule_AddInfoNotification(text);
    if (result != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("AddInfoNotification returned %s (%d)",
                                NotificationModule_GetStatusStr(result),
                                result);
    }
}

void showAutolaunchingNotification(const char *text)
{
    if (!gAutolaunchingNotifEnabled) {
        return;
    }

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                       NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT,
                                       gAutolaunchingNotifDurationShort ? 2.45f : 5.00f);

    NotificationModuleStatus result = NotificationModule_AddInfoNotification(text);
    if (result != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("AddInfoNotification returned %s (%d)",
                                NotificationModule_GetStatusStr(result),
                                result);
    }
}
