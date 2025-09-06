#include "battery_rp2xxx_simple.h"

/*
 * ============================================================================
 * Picowalker Driver Functions
 * ============================================================================
 */

 
/********************************************************************************
 * @brief           Gets current voltage from battery
 * @param N/A
 * @return pw_battery_status_t
********************************************************************************/
pw_battery_status_t pw_power_get_battery_status()
{
    uint16_t adc_raw = adc_read();
     
    // Convert to voltage (3.3V reference, 12-bit ADC)
    const float conversion_factor = 3.3f / (1 << 12) * 3;
    float voltage = adc_raw * conversion_factor; //3.3f / 4095.0f * 2.0f; // *2 for voltage divider
     
    pw_battery_status_t status;
    // Convert voltage to percentage (Li-ion: 3.0V-4.2V range)
    if (voltage >= 4.2f) status.percent = 100;
    else if (voltage <= 3.0f) status.percent = 0;
    else status.percent = (uint8_t)((voltage - 3.0f) / 1.2f * 100);     
    status.flags = 0;
    if (voltage < 3.2f) status.flags |= PW_BATTERY_STATUS_FLAGS_FAULT;
          
    return status;
}

/********************************************************************************
 * @brief           Battery Shutdown
 * @param N/A
********************************************************************************/
void pw_battery_shutdown()
{
    // TODO
}