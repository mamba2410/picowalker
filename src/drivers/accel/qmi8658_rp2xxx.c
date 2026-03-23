#include "qmi8658_rp2xxx.h"
#include "math.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"

struct QMI8658_Config qmi8658_config;
struct QMI8658_PedoConfig pedo_config;

// Hardware Pedometer Engine Step Counting variables
static volatile bool pedometer_data_ready = false;
static uint32_t last_read_steps = 0;
static uint32_t add_steps = 0;

// Software Pedometer Engine variables (mimics hardware engine)
uint32_t accumulated_steps = 0;
static unsigned int previous_hardware_steps = 0;
static struct repeating_timer step_timer;
static uint32_t timer_callback_ms = 20; // get as close to QMI8658_AccOdr_62_5Hz sample rating
const uint32_t min_step_interval_ms = 1000; // Minimum time between steps
#define MAX_SAMPLES 50
static float accel_history[MAX_SAMPLES];
static uint8_t history_index = 0;
static bool history_filled = false;
static uint32_t last_step_time = 0;
static uint8_t consecutive_signals = 0;
static bool in_step_motion = false;
static uint32_t step_motion_start = 0;

/********************************************************************************
 * @brief           Timer callback to mimic hardware pedometer engine
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

    // Software pedometer using pedo_config parameters
    float accel[3];
    QMI8658_Read_Acc_XYZ(accel);

    // Calculate magnitude (total acceleration)
    float magnitude = sqrtf(accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2]);

    // Sliding continuous buffer history
    accel_history[history_index] = magnitude;
    history_index = (history_index + 1) % pedo_config.sample_count;
    if (history_index == 0) history_filled = true;
    if (!history_filled) return true;

    // Find crest and trough in the sample window
    float crest = accel_history[0];
    float trough = accel_history[0];
    for (uint8_t i = 1; i < pedo_config.sample_count; i++)
    {
        if (accel_history[i] > crest) crest = accel_history[i];
        if (accel_history[i] < trough) trough = accel_history[i];
    }

    // Check if current motion qualifies as a step signal
    float peak2peak = crest - trough;
    float peak_deviation = fabsf(crest - 9.8f); // against a baseline gravity
    bool is_step_signal = (peak2peak > (pedo_config.fix_peak2peak / 100.0f)) && 
                          (peak_deviation > (pedo_config.fix_peak / 100.0f));
    uint32_t time_since_last_step = current_time - last_step_time;

    // Don't start new motion until minimum interval has passed since last step
    if (is_step_signal && !in_step_motion && time_since_last_step >= min_step_interval_ms)
    {
        in_step_motion = true;
        step_motion_start = current_time;
        consecutive_signals = 1;
    }
    else if (is_step_signal && in_step_motion)
    {
        consecutive_signals++;
        uint32_t motion_duration = current_time - step_motion_start;

        // Check if we have enough signals to confirm a step
        if (consecutive_signals >= pedo_config.signal_count &&
            motion_duration >= pedo_config.time_low &&
            motion_duration <= pedo_config.time_up)
        {
            accumulated_steps++;
            last_step_time = current_time;
            in_step_motion = false;
            consecutive_signals = 0;

            printf("[Debug] Software: Step detected! Total: %u, P2P: %.2f, Peak: %.2f, Signals: %u, Duration: %ums\n",
                   accumulated_steps, peak2peak, peak_deviation, pedo_config.signal_count, motion_duration);
        }
        else if (motion_duration > pedo_config.time_up)
        {
            // Motion too long - reset
            in_step_motion = false;
            consecutive_signals = 0;
        }
    }
    else if (!is_step_signal && in_step_motion)
    {
        // Motion ended - reset
        in_step_motion = false;
        consecutive_signals = 0;
    }

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
    // Original RP2040 settings (kept for reference)
    pedo_config.sample_count = 50;
    pedo_config.fix_peak2peak = 200;
    pedo_config.fix_peak = 100;
    pedo_config.time_up = 200;
    pedo_config.time_low = 20;
    pedo_config.time_count_entry = 10;
    pedo_config.fix_precision = 0;
    pedo_config.signal_count = 4;

    // More sensitive settings for RP2350
    // pedo_config.sample_count = 80;          // Increased from 50
    // pedo_config.fix_peak2peak = 300;        // Increased from 200
    // pedo_config.fix_peak = 150;             // Increased from 100
    // pedo_config.time_up = 250;              // Increased from 200
    // pedo_config.time_low = 15;              // Decreased from 20
    // pedo_config.time_count_entry = 8;       // Decreased from 10
    // pedo_config.fix_precision = 0;          // Keep same
    // pedo_config.signal_count = 3;  
    
    qmi8658_config.pedoConfig = pedo_config;

    QMI8658_init(qmi8658_config);
    QMI8658_Config_Pedometer_Interrupt();

    // IRQ Config
    gpio_init(DOF_INT1);
    gpio_set_dir(DOF_INT1, GPIO_IN);
    gpio_pull_down(DOF_INT1);

    // Enable interrupt on rising edge (when QMI8658 sets INT1 high)
    gpio_set_irq_enabled_with_callback(DOF_INT1, GPIO_IRQ_EDGE_RISE, true, &accel_irq_callback);

#if !PEDOMETER_ENGINE
    // Get initial hardware step count
    QMI8658_Read_Step_Count(&previous_hardware_steps);
    
    // Start timer for step processing
    add_repeating_timer_ms(timer_callback_ms, step_processing_timer_callback, NULL, &step_timer);
#endif
}

/********************************************************************************
 * @brief           Accel Sleep - Reduce power consumption
 * @param N/A
********************************************************************************/
void pw_accel_sleep()
{
#if !PEDOMETER_ENGINE
    // Cancel step processing timer to save power
    cancel_repeating_timer(&step_timer);
#endif
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
#if !PEDOMETER_ENGINE
    history_filled = false;
    history_index = 0;
    add_repeating_timer_ms(timer_callback_ms, step_processing_timer_callback, NULL, &step_timer);
#endif
    printf("[Debug] Accelerometer wake up - timer restarted\n");
}

/********************************************************************************
 * @brief           Accel Get New Steps - Returns accumulated steps from timer
 * @param N/A
 * @return uint32_t Number of new steps since last call
********************************************************************************/
uint32_t pw_accel_get_new_steps()
{
#if !PEDOMETER_ENGINE
    static uint32_t steps_at_last_call = 0;
    
    // Return new steps since last call (timer accumulates them in background)
    uint32_t new_steps = accumulated_steps - steps_at_last_call;
    steps_at_last_call = accumulated_steps;
    
    if (new_steps > 0) printf("[Debug] Returning %u new steps (total: %u)\n", new_steps, accumulated_steps);
#else
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

    // polling method for now...since interrupts are not working properly
    uint32_t current_hardware_steps;
    QMI8658_Read_Step_Count(&current_hardware_steps);
    if (current_hardware_steps > last_read_steps) new_steps = current_hardware_steps - last_read_steps;
    else new_steps = current_hardware_steps;
    last_read_steps = current_hardware_steps;

    // printf("[Pedometer] Read %u new steps (total: %u)\n", new_steps, current_hardware_steps);
    // // Check if pedometer has data
    // uint8_t status1 = QMI8658_Read_Status1();
    // printf("[Debug] STATUS1: 0x%02x (bit4=%d = pedometer interrupt)\n", status1, (status1 >> 4) & 1);

    // bool pin_state = gpio_get(DOF_INT1);
    // printf("DOF_INT1 initial state: %s\n", pin_state ? "HIGH" : "LOW");

    // // Read chip ID and revision
    // unsigned char chip_id, revision_id;
    // QMI8658_I2C_Read_Buffer(0x00, &chip_id, 1);     // WhoAmI register
    // QMI8658_I2C_Read_Buffer(0x01, &revision_id, 1); // Revision register
    // printf("Chip ID: 0x%02x, Revision: 0x%02x\n", chip_id, revision_id);

    // unsigned char ctrl_regs[8];
    // QMI8658_I2C_Read_Buffer(QMI8658_Register_Ctrl1, ctrl_regs, 8);
    // printf("\n=== QMI8658 Registers ===\n");
    // printf("CTRL1: 0x%02x\n", ctrl_regs[0]);
    // printf("CTRL2: 0x%02x (should be 0x07 for 2g@62.5Hz)\n", ctrl_regs[1]);
    // printf("CTRL3: 0x%02x\n", ctrl_regs[2]);
    // printf("CTRL4: 0x%02x\n", ctrl_regs[3]);
    // printf("CTRL5: 0x%02x\n", ctrl_regs[4]);
    // printf("CTRL6: 0x%02x\n", ctrl_regs[5]);
    // printf("CTRL7: 0x%02x (should be 0x01 for ACC only)\n", ctrl_regs[6]);
    // printf("CTRL8: 0x%02x (should be 0xd8 for pedo enabled)\n", ctrl_regs[7]);
    // printf("================================\n");

    // // Read accelerometer multiple times to see if data is changing
    // for (int i = 0; i < 5; i++) 
    // {
    //     float acc[3];
    //     QMI8658_Read_Acc_XYZ(acc);
    //     printf("[TEST %d] Accel: X=%.2f Y=%.2f Z=%.2f m/s²\n", i, acc[0], acc[1], acc[2]);
    //     sleep_ms(100);
    // }
#endif

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
    last_read_steps = 0;

    // Reset Software Pedometer Engine
    history_index = 0;
    history_filled = false;
    last_step_time = 0;
    consecutive_signals = 0;
    in_step_motion = false;
    step_motion_start = 0;
    for (uint8_t i = 0; i < MAX_SAMPLES; i++) 
    {
        accel_history[i] = 0.0f;
    }

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