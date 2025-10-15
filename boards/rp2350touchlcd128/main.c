#include "pico/time.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

#include "bsp/board_api.h"
#include "tusb.h"

#include "lvgl.h"

// Picowalker
#include "picowalker-defs.h"

#include "board_resources.h"

static bool is_spi_init = false;
static bool is_i2c_init = false;

// LVGL
static struct repeating_timer lvgl_timer;

extern void (*current_loop)(void);

bool power_sleep_enabled;

/********************************************************************************
 * @brief       Intialize SPI
 * @param N/A
********************************************************************************/
void board_spi_init()
{
    if (is_spi_init) return;

    // SPI Configuration
	spi_init(SCREEN_SPI_PORT, 270000 * 1000);
    gpio_set_function(SCREEN_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SCREEN_MOSI_PIN, GPIO_FUNC_SPI);

    is_spi_init = true;
}

/********************************************************************************
 * @brief       Intialize I2C
 * @param N/A
********************************************************************************/
void board_i2c_init()
{
    if (is_i2c_init) return;

    // I2C Config
    i2c_init(SENSOR_I2C_PORT, 400 * 1000);
    gpio_set_function(SENSOR_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SENSOR_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SENSOR_SDA_PIN);
    gpio_pull_up(SENSOR_SCL_PIN);

    is_i2c_init = true;
}

/********************************************************************************
 * @brief       Repeating Tick Increment
 * @param timer LVGL Struct Repeating Timer
********************************************************************************/
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer)
{
    lv_tick_inc(5);
    return true;
}

/********************************************************************************
 * @brief       Mount Callback
 * @param N/A
********************************************************************************/
void tud_mount_cb(void) 
{
    printf("[Info ] tusb mounted\n");

    // Halt sleep timer so we don't mess up tusb
    power_sleep_enabled = false;

}

/********************************************************************************
 * @brief       UnMount Callback
 * @param N/A
********************************************************************************/
void tud_umount_cb(void) 
{
    printf("[Info ] tusb unmounted\n");

    // Unmount and now we're safe to sleep
    power_sleep_enabled = true;
    //set_user_idle_timer(); //power_pico....
}

/********************************************************************************
 * @brief                   Suspend Callback
 * @param remote_wakeup_en 
********************************************************************************/
void tud_suspend_cb(bool remote_wakeup_en) 
{ 
    (void)remote_wakeup_en; 
}

/********************************************************************************
 * @brief       Resume Callback
 * @param N/A
********************************************************************************/
void tud_resume_cb(void) 
{

}

/********************************************************************************
 * @brief       CDC Task
 * @param N/A
********************************************************************************/
void cdc_task(void) 
{
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  // {
    // connected and there are data available
    if (tud_cdc_available()) 
    {
      // read data
      char buf[64];
      uint32_t count = tud_cdc_read(buf, sizeof(buf));
      (void) count;

      // Echo back
      // Note: Skip echo by commenting out write() and write_flush()
      // for throughput test e.g
      //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
      tud_cdc_write(buf, count);
      tud_cdc_write_flush();
      printf(buf);
    }
  // }
}

/********************************************************************************
 * @brief       Invoked when cdc when line state changed e.g connected/disconnected
 * @param itf
 * @param dtr
 * @param rts
********************************************************************************/
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) 
{
    printf("[CDC%d] Line state: DTR=%d RTS=%d\n", itf, dtr, rts);

    // TODO set some indicator
    if (dtr)printf("[CDC%d] Terminal connected\n", itf);
    else printf("[CDC%d] Terminal disconnected\n", itf);
}

/********************************************************************************
 * @brief       Invoked when CDC interface received data from host
 * @param itf
********************************************************************************/
void tud_cdc_rx_cb(uint8_t itf)
{
    uint32_t avail = tud_cdc_n_available(itf);
    printf("[CDC%d] RX callback: %lu bytes available\n", itf, avail);

    // DON'T read the data here! Just log that it arrived.
    // The IR driver (pw_ir_read) will consume it via service_usb_cdc()
    if (avail > 0 && itf == 1) printf("[CDC1] IR data ready for pw_ir_read()\n");
}

/********************************************************************************
 * @brief	      Main Function
 * @param N/A
********************************************************************************/
int main() 
{
    // Clock Config
    set_sys_clock_khz(PLL_SYS_KHZ, true);
    clock_configure(
        clk_peri,
        0,                                                
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 
        PLL_SYS_KHZ * 1000,                               
        PLL_SYS_KHZ * 1000                              
    );

    stdio_init_all();

    // I2C Config
    board_i2c_init();

    lv_init();
    add_repeating_timer_ms(5,   repeating_lvgl_timer_callback,       NULL, &lvgl_timer);

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };

    board_init();
    tusb_init(BOARD_TUD_RHPORT, &dev_init); // TODO Expanded Define...needs review

    walker_setup();

    while(true)
    {
        tud_task();
        lv_task_handler();
        current_loop();
        // cdc_task(); // Disabled - conflicts with IR USB CDC driver
    }

    return 0;
}