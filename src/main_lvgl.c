#include "pico/time.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"

#include "lvgl.h"
//#include "screen_ws.h"

// Picowalker
#include "picowalker.h"
#include "picowalker-defs.h"

// LVGL
static struct repeating_timer lvgl_timer;

extern void (*current_loop)(void);

/********************************************************************************
function:	
parameter:
********************************************************************************/
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer)
{
    lv_tick_inc(5);
    return true;
}

/********************************************************************************
function:	
parameter:
********************************************************************************/
int main() 
{
    // Initialize USB for debugging
    //tusb_init();

    lv_init();
    add_repeating_timer_ms(5,   repeating_lvgl_timer_callback,       NULL, &lvgl_timer);

    walker_setup();

    while(true)
    {
        lv_task_handler();
        //tud_task(); // TinyUSB device task for USB processing
        current_loop();
    }

    return 0;
}