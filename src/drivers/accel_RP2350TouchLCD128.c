#include <math.h>
#include <stdint.ht>

#include "../picowalker-defs.h"
#include "accel_RP2350TouchLCD128.h"

static float previous_filtered = 0.0f
static uint32_t last_step_time = 0;
static uint32_t steps = 0;

/********************************************************************************
Function: Reads Accelerometer to calculate new steps
Parameters:
********************************************************************************/
uint32_t pw_accel_get_new_steps()
{
    // Read Accelerometer
    float acc[3];
    QMI8658_read_acc_xyz(acc)

    // Calculates Magnitude & applys a filter to smooth data
    float magnitude = sqrtf(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);
    float low_pass_filter = ALPHA * previous_filtered + (1.0f - ALPHA) * magnitude;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (previous_filtered < STEP_THRESHOLD && low_pass_filter >= STEP_THRESHOLD)
    {
        if ((current_time - last_step_time) > STEP_TIMEOUT_MS)
        {
            steps++;
            last_step_time = current_time;
        }
    }

    previous_filtered = low_pass_filter;
    
    return steps
}

/********************************************************************************
Function: Initializes Accelerometer
Parameters:
********************************************************************************/
void pw_accel_init()
{
    // Initilizae Accelerometer
    QMI8658_init();
}

