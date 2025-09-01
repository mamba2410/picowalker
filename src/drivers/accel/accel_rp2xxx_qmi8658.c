#include "accel_rp2xxx_qmi8658.h"
#include "math.h"

/*
 * ============================================================================
 * Picowalker Driver Functions
 * ============================================================================
 */

 
/********************************************************************************
 * @brief           Accel Initialize
 * @param N/A
********************************************************************************/
void pw_accel_init()
{
    // Initialize IMU - Accel
    QMI8658_init();
}

/********************************************************************************
 * @brief           Accel Sleep
 * @param N/A
********************************************************************************/
void pw_accel_sleep()
{
    QMI8658_enableSensors(QMI8658_CTRL7_DISABLE_ALL);
}

/********************************************************************************
 * @brief           Accel Wake up
 * @param N/A
********************************************************************************/
void pw_accel_wake()
{
    QMI8658_enableSensors(QMI8658_CTRL7_ACC_ENABLE);
}

/********************************************************************************
 * @brief           Accel Get New Steps
 * @param N/A
 * @return uint32_t
********************************************************************************/
uint32_t pw_accel_get_new_steps()
{
    float accel[3];
    QMI8658_read_acc_xyz(accel);

    // Calculate magnitude
    float magnitude = sqrtf(accel[0]*accel[0] + accel[1]*accel[1] + accel[2]*accel[2]);

    // Simple peak detection
    static float threshold = 1.2f;
    static uint64_t min_step_interval = 300000;
    static float prev_magnitude = 0;
    static uint64_t last_step_time = 0;

    uint64_t now = time_us_64();

    // Detect step: magnitude crosses threshold upward
      if (magnitude > threshold && prev_magnitude <= threshold) {
          if (now - last_step_time > min_step_interval) {
              //last_step_time = now;
              //prev_magnitude = magnitude;
              return 1;
          }
      }

      //prev_magnitude = magnitude;
      return 0;
}