#include "power_rp2xxx_ws.h"

static volatile bool power_should_sleep;
pw_wake_reason_t wake_reason;

// TODO will probably need extern variables to signal LVGL code to turn of screen or sleep

/********************************************************************************
 * @brief           Power Initialize
 * @param N/A
********************************************************************************/
void pw_power_init()
{
    // Initialize Device
    WS_Module_Init();
}

/********************************************************************************
 * @brief           Power Enter Sleep
 * @param N/A
********************************************************************************/
void pw_power_enter_sleep()
{
    // TODO Enter Sleep Code
}

/********************************************************************************
 * @brief           Power Should Sleep
 * @param N/A
********************************************************************************/
bool pw_power_should_sleep()
{
    return power_should_sleep;
}

/********************************************************************************
 * @brief           Power Get Awake Reason
 * @param N/A
 * @return pw_wake_reason_t
********************************************************************************/
pw_wake_reason_t pw_power_get_wake_reason() 
{
    return wake_reason;
}

