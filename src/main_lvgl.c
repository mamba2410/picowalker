#include "pico/time.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

#include <tusb.h>
//#include "bsp/board_api.h"
//#include "tusb_config.h"

#include "lvgl.h"
//#include "screen_ws.h"

// Picowalker
#include "picowalker.h"
#include "picowalker-defs.h"
//#include "msc_disk.h"

// LVGL
static struct repeating_timer lvgl_timer;

extern void (*current_loop)(void);

bool power_sleep_enabled;

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
void tud_mount_cb(void) {
    printf("[Info ] tusb mounted\n");

    // Halt sleep timer so we don't mess up tusb
    power_sleep_enabled = false;

}

/********************************************************************************
 * @brief       UnMount Callback
 * @param N/A
********************************************************************************/
void tud_umount_cb(void) {
    printf("[Info ] tusb unmounted\n");

    // Unmount and now we're safe to sleep
    power_sleep_enabled = true;
    //set_user_idle_timer(); //power_pico....
}

/********************************************************************************
 * @brief                   Suspend Callback
 * @param remote_wakeup_en 
********************************************************************************/
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }

/********************************************************************************
 * @brief       Resume Callback
 * @param N/A
********************************************************************************/
void tud_resume_cb(void) {}

/********************************************************************************
 * @brief       CDC Task
 * @param N/A
********************************************************************************/
void cdc_task(void) {
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  {
    // connected and there are data available
    if (tud_cdc_available()) {
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
  }
}

/********************************************************************************
 * @brief       Invoked when cdc when line state changed e.g connected/disconnected
 * @param itf
 * @param dtr
 * @param rts
********************************************************************************/
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if (dtr) {
    // Terminal connected
  } else {
    // Terminal disconnected
  }
}

/********************************************************************************
 * @brief       Invoked when CDC interface received data from host
 * @param itf
********************************************************************************/
void tud_cdc_rx_cb(uint8_t itf) {
  (void) itf;
}

/********************************************************************************
 * @brief	      Main Function
 * @param N/A
********************************************************************************/
int main() 
{

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
        cdc_task();
    }

    return 0;
}