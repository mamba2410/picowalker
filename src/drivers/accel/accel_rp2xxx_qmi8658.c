#include "accel_rp2xxx_qmi8658.h"
#include "math.h"
<<<<<<< HEAD
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"

struct QMI8658_Config qmi8658_config;
struct QMI8658_PedoConfig pedo_config;

// Step counting variables
uint32_t accumulated_steps = 0;  // Global for external access
static uint32_t previous_hardware_steps = 0;
static struct repeating_timer step_timer;

// Software fallback variables
static float prev_magnitude = 0.0f;
static uint32_t last_step_time = 0;
static float step_threshold = 1.2f;
static uint32_t min_step_interval_ms = 300;
static float magnitude_filter = 0.0f;
static const float filter_alpha = 0.3f;

/********************************************************************************
 * @brief           Timer callback to continuously process steps
 * @param timer     Repeating timer struct
 * @return bool     true to continue timer
********************************************************************************/
static bool step_processing_timer_callback(struct repeating_timer *timer)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Try hardware pedometer first
    unsigned int current_hardware_steps = 0;
    QMI8658_read_step_count(&current_hardware_steps);
    
    if (current_hardware_steps > previous_hardware_steps) {
        // Hardware pedometer is working
        uint32_t new_hardware_steps = current_hardware_steps - previous_hardware_steps;
        accumulated_steps += new_hardware_steps;
        previous_hardware_steps = current_hardware_steps;
        
        printf("[Debug] Hardware: +%u steps (total: %u)\n", new_hardware_steps, accumulated_steps);
        return true;
    }
    
    // Fallback to software detection if hardware isn't working
    float accel[3];
    QMI8658_read_acc_xyz(accel);
    
    float magnitude = sqrtf(accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2]);
    magnitude_filter = (filter_alpha * magnitude) + ((1.0f - filter_alpha) * magnitude_filter);
    float magnitude_diff = magnitude_filter - prev_magnitude;
    
    if (magnitude_diff > step_threshold && 
        (current_time - last_step_time) > min_step_interval_ms) {
        
        accumulated_steps++;
        last_step_time = current_time;
        
        printf("[Debug] Software: Step detected! Total: %u, Mag: %.2f\n", 
               accumulated_steps, magnitude_filter);
    }
    
    prev_magnitude = magnitude_filter;
    return true;
}
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)

/*
 * ============================================================================
 * Picowalker Driver Functions
 * ============================================================================
 */

<<<<<<< HEAD
/********************************************************************************
 * @brief           Accel Initialize with Hardware Pedometer + Software Fallback
=======
 
/********************************************************************************
 * @brief           Accel Initialize
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
 * @param N/A
********************************************************************************/
void pw_accel_init()
{
<<<<<<< HEAD
    // Initialize hardware pedometer configuration (original settings)
    qmi8658_config.inputSelection = QMI8658_CONFIG_ACC_ENABLE;
    qmi8658_config.accRange = QMI8658_AccRange_4g;
    qmi8658_config.accOdr = QMI8658_AccOdr_125Hz;
    qmi8658_config.gyrRange = QMI8658_GyrRange_512dps;
    qmi8658_config.gyrOdr = QMI8658_GyrOdr_1000Hz;
    qmi8658_config.magDev = QMI8658_MagDev_AKM09918;
    qmi8658_config.magOdr = QMI8658_MagOdr_125Hz;
    qmi8658_config.aeOdr = QMI8658_AeOdr_128Hz;
    
    qmi8658_config.enablePedometer = QMI8658_PedoMode_Enable;
    pedo_config.sample_count = 50;
    pedo_config.fix_peak2peak = 200;
    pedo_config.fix_peak = 100;
    pedo_config.time_up = 200;
    pedo_config.time_low = 20;
    pedo_config.time_count_entry = 10;
    pedo_config.fix_precision = 0;
    pedo_config.signal_count = 4;
    
    qmi8658_config.pedoConfig = pedo_config;

    QMI8658_init(qmi8658_config);
    
    // Initialize step counting variables
    accumulated_steps = 0;
    previous_hardware_steps = 0;
    
    // Initialize software fallback variables
    prev_magnitude = 0.0f;
    last_step_time = 0;
    magnitude_filter = 0.0f;
    
    // Get initial hardware step count
    QMI8658_read_step_count(&previous_hardware_steps);
    
    // Start timer for continuous step processing (100ms interval)
    add_repeating_timer_ms(100, step_processing_timer_callback, NULL, &step_timer);
    
    printf("[Debug] Pedometer initialized - Hardware + Software fallback, timer started\n");
}

/********************************************************************************
 * @brief           Accel Sleep - Reduce power consumption
=======
    // Initialize IMU - Accel
    QMI8658_init();
}

/********************************************************************************
 * @brief           Accel Sleep
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
 * @param N/A
********************************************************************************/
void pw_accel_sleep()
{
<<<<<<< HEAD
    // Cancel step processing timer to save power
    cancel_repeating_timer(&step_timer);
    // Keep accelerometer enabled for hardware pedometer
    QMI8658_enable_sensors(QMI8658_CTRL7_ACC_ENABLE);
    printf("[Debug] Accelerometer sleeping - timer stopped, hardware pedometer active\n");
}

/********************************************************************************
 * @brief           Accel Wake up - Resume normal operation
=======
    QMI8658_enableSensors(QMI8658_CTRL7_DISABLE_ALL);
}

/********************************************************************************
 * @brief           Accel Wake up
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
 * @param N/A
********************************************************************************/
void pw_accel_wake()
{
<<<<<<< HEAD
    // Re-enable accelerometer and restart step processing timer
    QMI8658_enable_sensors(QMI8658_CTRL7_ACC_ENABLE);
    add_repeating_timer_ms(100, step_processing_timer_callback, NULL, &step_timer);
    printf("[Debug] Accelerometer wake up - timer restarted\n");
}

/********************************************************************************
 * @brief           Accel Get New Steps - Returns accumulated steps from timer
 * @param N/A
 * @return uint32_t Number of new steps since last call
********************************************************************************/
uint32_t pw_accel_get_new_steps()
{
    static uint32_t steps_at_last_call = 0;
    
    // Return new steps since last call (timer accumulates them in background)
    uint32_t new_steps = accumulated_steps - steps_at_last_call;
    steps_at_last_call = accumulated_steps;
    
    if (new_steps > 0) {
        printf("[Debug] Returning %u new steps (total: %u)\n", new_steps, accumulated_steps);
    }
    
    return new_steps;
}

/********************************************************************************
 * @brief           Reset Step Counter
 * @param N/A
********************************************************************************/
void pw_accel_reset_steps()
{    
    accumulated_steps = 0;
    previous_hardware_steps = 0;
    
    // Reset software fallback variables
    prev_magnitude = 0.0f;
    last_step_time = 0;
    magnitude_filter = 0.0f;
    
    // Try to reset hardware counter
    QMI8658_reset_step_count();
    QMI8658_read_step_count(&previous_hardware_steps);
    
    printf("[Debug] Step counter reset - Hardware + Software\n");
}

/********************************************************************************
 * @brief           Add manual steps (for canvas press simulation)
 * @param steps     Number of steps to add
********************************************************************************/
void pw_accel_add_steps(uint32_t steps)
{
    accumulated_steps += steps;
    printf("[Debug] Added %u manual steps (total: %u)\n", steps, accumulated_steps);
=======
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
    uint64_t now = time_us_64();
    float prev_magnitude = 0;
    uint64_t last_step_time = 0;
    
    // Detect step: magnitude crosses threshold upward
      if (magnitude > threshold && prev_magnitude <= threshold) {
          if (now - last_step_time > min_step_interval) {
              last_step_time = now;
              prev_magnitude = magnitude;
              return 1;
          }
      }

      prev_magnitude = magnitude;
      return 0;
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
}