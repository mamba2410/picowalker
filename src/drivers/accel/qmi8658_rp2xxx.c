#include "qmi8658_rp2xxx.h"
#include "math.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"

struct QMI8658_Config qmi8658_config;
struct QMI8658_PedoConfig pedo_config;

// Pedometer Step Counting variables
static volatile bool pedometer_data_ready = false;
static uint32_t last_read_steps = 0;
static uint32_t add_steps = 0;

// Software fallback variables
uint32_t accumulated_steps = 0;
static unsigned int previous_hardware_steps = 0;
static struct repeating_timer step_timer;
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
    QMI8658_Read_Step_Count(&current_hardware_steps);
    
    if (current_hardware_steps > previous_hardware_steps) 
    {
        // Hardware pedometer is working
        uint32_t new_hardware_steps = current_hardware_steps - previous_hardware_steps;
        accumulated_steps += new_hardware_steps;
        previous_hardware_steps = current_hardware_steps;
        
        printf("[Debug] Hardware: +%u steps (total: %u)\n", new_hardware_steps, accumulated_steps);
        return true;
    }
    
    // Fallback to software detection if hardware isn't working
    float accel[3];
    QMI8658_Read_Acc_XYZ(accel);
    
    float magnitude = sqrtf(accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2]);
    magnitude_filter = (filter_alpha * magnitude) + ((1.0f - filter_alpha) * magnitude_filter);
    float magnitude_diff = magnitude_filter - prev_magnitude;
    
    if (magnitude_diff > step_threshold && 
        (current_time - last_step_time) > min_step_interval_ms) 
    {
        
        accumulated_steps++;
        last_step_time = current_time;
        
        printf("[Debug] Software: Step detected! Total: %u, Mag: %.2f\n", 
               accumulated_steps, magnitude_filter);
    }
    
    prev_magnitude = magnitude_filter;
    return true;
}

/********************************************************************************
 * @brief           Accel IRQ Callback
 * @param gpio      Signal PIN
 * @param events    Events from LVGL
********************************************************************************/
static void accel_irq_callback(uint gpio, uint32_t events)
{
    if (gpio == DOF_INT1)
    {
        pedometer_data_ready = true;
    }
}

/*
 * ============================================================================
 * Picowalker Driver Functions
 * ============================================================================
 */

/********************************************************************************
 * @brief           Accel Initialize with Hardware Pedometer + Software Fallback
 * @param N/A
********************************************************************************/
void pw_accel_init()
{
    // Initialize hardware pedometer configuration (original settings)
    qmi8658_config.inputSelection = QMI8658_CONFIG_ACC_ENABLE;
    qmi8658_config.accRange = QMI8658_AccRange_2g;
    qmi8658_config.accOdr = QMI8658_AccOdr_62_5Hz;
    qmi8658_config.gyrRange = QMI8658_GyrRange_512dps;
    qmi8658_config.gyrOdr = QMI8658_GyrOdr_1000Hz;
    qmi8658_config.magDev = QMI8658_MagDev_AKM09918;
    qmi8658_config.magOdr = QMI8658_MagOdr_125Hz;
    qmi8658_config.aeOdr = QMI8658_AeOdr_128Hz;
    
    qmi8658_config.enablePedometer = 1;
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
    QMI8658_Config_Pedometer_Interrupt();
    
    // IRQ Config
    gpio_init(DOF_INT1);
    gpio_set_dir(DOF_INT1, GPIO_IN);
    gpio_pull_down(DOF_INT1);

    // Enable interrupt on rising edge (when QMI8658 sets INT1 high)
    gpio_set_irq_enabled_with_callback(DOF_INT1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &accel_irq_callback);

    // // Get initial hardware step count
    // QMI8658_Read_Step_Count(&previous_hardware_steps);
    
    // // Start timer for continuous step processing (100ms interval)
    // add_repeating_timer_ms(100, step_processing_timer_callback, NULL, &step_timer);
}

/********************************************************************************
 * @brief           Accel Sleep - Reduce power consumption
 * @param N/A
********************************************************************************/
void pw_accel_sleep()
{
    // Cancel step processing timer to save power
    // cancel_repeating_timer(&step_timer);

    // Keep accelerometer enabled for hardware pedometer
    QMI8658_Enable_Sensors(QMI8658_CTRL7_ACC_ENABLE);
    printf("[Debug] Accelerometer sleeping - timer stopped, hardware pedometer active\n");
}

/********************************************************************************
 * @brief           Accel Wake up - Resume normal operation
 * @param N/A
********************************************************************************/
void pw_accel_wake()
{
    // Re-enable accelerometer and restart step processing timer
    QMI8658_Enable_Sensors(QMI8658_CTRL7_ACC_ENABLE);
    // add_repeating_timer_ms(100, step_processing_timer_callback, NULL, &step_timer);
    printf("[Debug] Accelerometer wake up - timer restarted\n");
}

/********************************************************************************
 * @brief           Accel Get New Steps - Returns accumulated steps from timer
 * @param N/A
 * @return uint32_t Number of new steps since last call
********************************************************************************/
uint32_t pw_accel_get_new_steps()
{
    uint32_t new_steps = 0;
    // IRQ Callback not working?
    if (pedometer_data_ready)
    {
        uint32_t current_hardware_steps;
        QMI8658_Read_Step_Count(&current_hardware_steps);

        if (current_hardware_steps > last_read_steps) new_steps = current_hardware_steps - last_read_steps;
        else new_steps = current_hardware_steps;

        last_read_steps = current_hardware_steps;
        pedometer_data_ready = false;
        printf("[Pedometer] Read %u new steps (total: %u)\n", new_steps, current_hardware_steps);
    }
    // // Check if pedometer has data
    // uint8_t status1 = QMI8658_Read_Status1();
    // printf("[Debug] STATUS1: 0x%02x (bit4=%d = pedometer interrupt)\n", status1, (status1 >> 4) & 1);

    // bool pin_state = gpio_get(DOF_INT1);
    // printf("DOF_INT1 initial state: %s\n", pin_state ? "HIGH" : "LOW");

    // polling method for now...
    uint32_t current_hardware_steps;
    QMI8658_Read_Step_Count(&current_hardware_steps);
    if (current_hardware_steps > last_read_steps) new_steps = current_hardware_steps - last_read_steps;
    else new_steps = current_hardware_steps;
    last_read_steps = current_hardware_steps;

    // This is for adding steps manually (Cheating...mainly for debugging)
    if (add_steps > 0)
    {
        new_steps = add_steps;
        add_steps = 0;
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
    last_read_steps = 0;

    // Try to reset hardware counter
    QMI8658_Reset_Step_Count();
    QMI8658_Read_Step_Count(&previous_hardware_steps);
    
    printf("[Debug] Step counter reset - Hardware + Software\n");
}

/********************************************************************************
 * @brief           Add manual steps (for canvas press simulation)
 * @param steps     Number of steps to add
********************************************************************************/
void pw_accel_add_steps(uint32_t steps)
{
    add_steps += steps;
    printf("[Debug] Added %u manual steps (total: %u)\n", steps, add_steps);
}