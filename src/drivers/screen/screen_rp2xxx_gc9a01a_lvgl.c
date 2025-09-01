#include "screen_rp2xxx_gc9a01a_lvgl.h"
#include "battery_rp2xxx_simple.h"
<<<<<<< HEAD
#include "accel_rp2xxx_qmi8658.h"
#include "picowalker-defs.h"

#ifdef __has_include
#if __has_include("board_resources.h")
#include "board_resources.h"
#endif
#endif

#ifndef PW_SPEAKER_PIN
#define PW_SPEAKER_PIN 16
#endif

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// LVGL Settings
static lv_disp_drv_t driver_display;
static lv_disp_draw_buf_t display_buffer;

#ifdef PICO_RP2040
#define LVGL_BUFFER_DIVISOR 2
static lv_color_t *buffer0;
static lv_color_t *buffer1;
#else
#define LVGL_BUFFER_DIVISOR 2
static lv_color_t buffer0[DISP_HOR_RES * DISP_VER_RES/LVGL_BUFFER_DIVISOR];
static lv_color_t buffer1[DISP_HOR_RES * DISP_VER_RES/LVGL_BUFFER_DIVISOR];
#endif
=======

#include <time.h>

// LVGL Settings
static lv_disp_draw_buf_t display_buffer;
static lv_color_t buffer0[DISP_HOR_RES * DISP_VER_RES/2];
static lv_color_t buffer1[DISP_HOR_RES * DISP_VER_RES/2];
static lv_disp_drv_t driver_display;
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)

static lv_indev_drv_t driver_touch;
static uint16_t touch_x;
static uint16_t touch_y;
static lv_indev_state_t touch_state;

static lv_group_t *tile_group;
static lv_obj_t *canvas;
static lv_color_t canvas_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];

<<<<<<< HEAD
static lv_obj_t *brightness_slider;
static lv_obj_t *brightness_label;

static lv_obj_t *battery_bar;
static lv_obj_t *battery_label;
static lv_obj_t *tile_view;

static struct repeating_timer lvgl_timer;
static struct repeating_timer battery_timer;
bool is_sleeping = false;

/********************************************************************************
 * @brief           Play simple beep sound using piezo buzzer
 * @param duration  Duration in milliseconds
 ********************************************************************************/
static void play_beep(uint32_t duration)
{
    // Turn on piezo at 4kHz (resonant frequency) with 50% duty cycle
    // With wrap=1000, 50% duty cycle = 500
    pwm_set_gpio_level(PW_SPEAKER_PIN, 500);
    sleep_ms(duration);
    
    // Turn off
    pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
}

/********************************************************************************
 * @brief           Play click sound for button presses
 ********************************************************************************/
static void play_click_sound()
{
    play_beep(50); // Short 50ms beep
}

/********************************************************************************
 * @brief           Play confirmation sound for actions
 ********************************************************************************/
static void play_confirm_sound()
{
    play_beep(100); // Longer 100ms beep
}

/********************************************************************************
=======
static struct repeating_timer lvgl_timer;
bool is_sleeping = false;

/********************************************************************************
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
 * @brief           LVGL Repeating Timer Callback used to pass a tick / time
 * @param timer     Repeating Timer Struct
 * @return bool
********************************************************************************/
static bool repeating_lvgl_timer_callback(struct repeating_timer *timer)
{
    lv_tick_inc(5);
    return true;
}

/********************************************************************************
 * @brief           Display Flush Callback
 * @param display   LVGL display driver
 * @param area      LVGL area buffer
 * @param color     LVGL color for area
********************************************************************************/
static void display_flush_callback(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *color)
{
    // Manually swap red and blue channels in RGB565 data
    uint16_t *pixel_data = (uint16_t*)color;
    uint32_t pixel_count = (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1);
    
    for(uint32_t i = 0; i < pixel_count; i++) {
        uint16_t pixel = pixel_data[i];
        pixel_data[i] = (pixel << 8) | (pixel >> 8);
    }

    GC9A01A_SetWindows(area->x1, area->y1, area->x2 , area->y2);
    dma_channel_configure(dma_tx,
                          &c,
                          &spi_get_hw(LCD_SPI_PORT)->dr, 
                          color,
                          //((area->x2 + 1 - area-> x1)*(area->y2 + 1 - area -> y1))*2,
                          pixel_count * 2,
                          true);
}

/********************************************************************************
 * @brief           Touch Callback
 * @param gpio      Signal PIN
 * @param events    Events from LVGL
********************************************************************************/
static void touch_callback(uint gpio, uint32_t events)
{
    if (gpio == TOUCH_INT_PIN)
    {
        CST816S_Get_Point();
        touch_x = Touch_CTS816.x_point;
        touch_y = Touch_CTS816.y_point;
        touch_state = LV_INDEV_STATE_PRESSED;
    }
}

/********************************************************************************
 * @brief           Read Touch Inputs
 * @param driver    LVGL Input Device Driver
 * @param data      Returns the data to the Input Device Driver
********************************************************************************/
static void touch_read_callback(lv_indev_drv_t *driver, lv_indev_data_t *data)
{
    data->point.x = touch_x;
    data->point.y = touch_y; 
    data->state = touch_state;
    touch_state = LV_INDEV_STATE_RELEASED;
}

/********************************************************************************
 * @brief           Left Button Callback
 * @param event     LVGL event call back
********************************************************************************/
static void button_left_callback(lv_event_t *event)
{
<<<<<<< HEAD
    play_click_sound();
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    pw_button_callback(BUTTON_L);
}

/********************************************************************************
 * @brief           Middle Button Callback
 * @param event     LVGL event call back
********************************************************************************/
static void button_middle_callback(lv_event_t *event)
{
<<<<<<< HEAD
    play_click_sound();
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    pw_button_callback(BUTTON_M);
}

/********************************************************************************
 * @brief           Right Button Callback
 * @param event     LVGL event call back
********************************************************************************/
static void button_right_callback(lv_event_t *event)
{
<<<<<<< HEAD
    play_click_sound();
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    pw_button_callback(BUTTON_R);
}

/********************************************************************************
 * @brief           Direct Memory Access Handler for Display
 * @param N/A
********************************************************************************/
static void direct_memory_access_handler(void)
{
    if (dma_channel_get_irq0_status(dma_tx)) {
        dma_channel_acknowledge_irq0(dma_tx);
        lv_disp_flush_ready(&driver_display); // Indicate you are ready with the flushing
    }
}

/********************************************************************************
 * @brief           Changes brightness of the screen
 * @param event     LVGL event from indev input
********************************************************************************/
static void brightness_slider_event_callback(lv_event_t * event)
{
      lv_obj_t *slider = lv_event_get_target(event);
      int32_t value = lv_slider_get_value(slider);
      WS_SET_PWM(value);
<<<<<<< HEAD
      
      static char brightness_text[32];
      snprintf(brightness_text, sizeof(brightness_text), "Brightness: %d%%", (int)value);
      lv_label_set_text(brightness_label, brightness_text);
}

/********************************************************************************
 * @brief           EEPROM Wipe Button Callback
 * @param event     LVGL event from button LONG press
********************************************************************************/
static void eeprom_wipe_button_callback(lv_event_t * event)
{
    if (event->code == LV_EVENT_LONG_PRESSED) {
        printf("[EEPROM] Wiping EEPROM via button LONG press...\n");
        
        // Wipe entire EEPROM to erased state (0xFF)
        pw_eeprom_set_area(0, 0xFF, 64 * 1024);
        
        printf("[EEPROM] EEPROM wiped successfully!\n");
        
        // Reset steps counter as well since EEPROM is wiped
        pw_accel_reset_steps();
    }
}

/********************************************************************************
 * @brief           EEPROM Save Button Callback
 * @param event     LVGL event from button press
********************************************************************************/
static void eeprom_save_button_callback(lv_event_t * event)
{
    if (event->code == LV_EVENT_PRESSED) {
        play_confirm_sound();
        printf("[EEPROM] Saving EEPROM via button press...\n");
        
        // Check if there are changes to save
        if (pw_eeprom_is_cache_dirty()) {
            if (pw_eeprom_flush_to_flash() == 0) {
                printf("[EEPROM] EEPROM saved successfully!\n");
            } else {
                printf("[EEPROM] Failed to save EEPROM!\n");
            }
        } else {
            printf("[EEPROM] No changes to save\n");
        }
    }
}

/********************************************************************************
 * @brief           Updates Battery Display
 * @param timer     Repeating Timer Struct
 * @return bool
********************************************************************************/
static bool repeating_battery_timer_callback(struct repeating_timer *timer)
{
    if (is_sleeping || !battery_bar) return true;
    
    // Read battery status
    pw_battery_status_t battery_status = pw_power_get_battery_status();
    
    // Update battery bar value with smooth animation
    lv_bar_set_value(battery_bar, battery_status.percent, LV_ANIM_ON);
    
    // Update battery label with percentage
    // static char battery_text[32];
    // snprintf(battery_text, sizeof(battery_text), "Battery: %d%%", battery_status.percent);
    // lv_label_set_text(battery_label, battery_text);
    if (battery_label) {
        static char battery_text[32];
        snprintf(battery_text, sizeof(battery_text), "Battery: %d%%", battery_status.percent);
        lv_label_set_text(battery_label, battery_text);
    }
    
    return true;
}

/********************************************************************************
 * @brief           Tileview Event Callback - triggered when tiles change
 * @param event     LVGL event from tileview
********************************************************************************/
static void tileview_event_callback(lv_event_t * event)
{
    lv_obj_t *tileview = lv_event_get_target(event);
    
    if (event->code == LV_EVENT_SCROLL_END) {
        // Force immediate battery update when tile changes
        repeating_battery_timer_callback(NULL);
        printf("[Debug] Tile changed - battery updated\n");
    }
}

/********************************************************************************
 * @brief           Canvas Press Callback - adds steps when canvas is pressed
 * @param event     LVGL event from canvas
********************************************************************************/
static void canvas_press_callback(lv_event_t * event)
{
    if (event->code == LV_EVENT_PRESSED) {
        // Add a step when canvas is pressed (simulates walking)
        pw_accel_add_steps(1);
        printf("[Debug] Canvas pressed - step added!\n");
    }
}

/********************************************************************************
 * @brief           Manual Battery Update (can be called externally)
 * @param N/A
********************************************************************************/
void pw_screen_update_battery()
{
    repeating_battery_timer_callback(NULL);
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
}

/*
 * ============================================================================
 * Picowalker Driver Functions
 * ============================================================================
 */


/********************************************************************************
 * @brief           Initialize the Picowalker Screen 
 * @param N/A
********************************************************************************/
void pw_screen_init() 
{
<<<<<<< HEAD
    // Initialize audio/buzzer for 4kHz piezo optimal frequency
    gpio_init(PW_SPEAKER_PIN);
    gpio_set_function(PW_SPEAKER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PW_SPEAKER_PIN);
    
    // Configure for 4kHz frequency: 125MHz / (31.25 * 1000) = 4kHz
    pwm_set_wrap(slice_num, 1000);           // Higher resolution
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);  // Start silent
    pwm_set_clkdiv(slice_num, 31.25f);       // 4kHz frequency for piezo resonance
    pwm_set_enabled(slice_num, true);

=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    // Initialize WaveShare 1.28" LCD - Screen
    WS_SET_PWM(10);
    GC9A01A_Init(HORIZONTAL);
    GC9A01A_Clear(BLACK);

<<<<<<< HEAD
#ifdef PICO_RP2040
    buffer0 = malloc((DISP_HOR_RES * DISP_VER_RES / 2) * sizeof(lv_color_t));
    buffer1 = malloc((DISP_HOR_RES * DISP_VER_RES / 2) * sizeof(lv_color_t));
#endif
    // Initialize LVGL Display
    lv_disp_draw_buf_init(&display_buffer, buffer0, buffer1, DISP_HOR_RES * DISP_VER_RES / LVGL_BUFFER_DIVISOR); 
=======
    // Initialize LVGL Display
    lv_disp_draw_buf_init(&display_buffer, buffer0, buffer1, DISP_HOR_RES * DISP_VER_RES / 2); 
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    lv_disp_drv_init(&driver_display);    
    driver_display.flush_cb = display_flush_callback;
    driver_display.draw_buf = &display_buffer;        
    driver_display.hor_res = DISP_HOR_RES;
    driver_display.ver_res = DISP_VER_RES;
    lv_disp_t *display = lv_disp_drv_register(&driver_display);
<<<<<<< HEAD
    // lv_disp_set_rotation(display, LV_DISP_ROT_90); // TODO
#if TOUCH
=======

>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    // Initialize Touch Screen - Button
    CST816S_init(CST816S_Point_Mode);

    // Initialize Touch Screen Input
    lv_indev_drv_init(&driver_touch); 
    driver_touch.type = LV_INDEV_TYPE_POINTER;    
    driver_touch.read_cb = touch_read_callback;            
    lv_indev_t * touch_screen = lv_indev_drv_register(&driver_touch);
    WS_IRQ_SET(TOUCH_INT_PIN, GPIO_IRQ_EDGE_RISE, &touch_callback);
<<<<<<< HEAD
#endif
=======

>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    // Initialize DMA Direct Memory Access
    dma_channel_set_irq0_enabled(dma_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, direct_memory_access_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Picowalker Screen
    lv_obj_t *screen = lv_scr_act();

    // Picowalker Tile
    tile_group = lv_group_create();
<<<<<<< HEAD
    tile_view = lv_tileview_create(screen);
    lv_obj_set_scrollbar_mode(tile_view,  LV_SCROLLBAR_MODE_OFF);
    lv_group_add_obj(tile_group, tile_view);
    
    // Add tileview event callback for tile changes
    lv_obj_add_event_cb(tile_view, tileview_event_callback, LV_EVENT_SCROLL_END, NULL);
    lv_obj_t *tile_picowalker = lv_tileview_add_tile(tile_view, 0, 0, LV_DIR_BOTTOM);

    // Pokeball Image ... I want to add more
    LV_IMG_DECLARE(picowalker_background);
    lv_obj_t *background = lv_img_create(tile_picowalker);
=======
    lv_obj_t *tile_view = lv_tileview_create(screen);
    lv_obj_set_scrollbar_mode(tile_view,  LV_SCROLLBAR_MODE_OFF);
    lv_group_add_obj(tile_group, tile_view);
    lv_obj_t *tile_picowalker = lv_tileview_add_tile(tile_view, 0, 0, LV_DIR_BOTTOM);

    // Pokeball Image ... I want to add more
    // LV_IMG_DECLARE(pokeball_240x240);
    LV_IMG_DECLARE(picowalker_background);
    lv_obj_t *background = lv_img_create(tile_picowalker);
    // lv_img_set_src(background, &pokeball_240x240);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    lv_img_set_src(background, &picowalker_background);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

    // Button Style Not Pressed
    static lv_style_t button_style_base;
    lv_style_init(&button_style_base);
    lv_style_set_radius(&button_style_base, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&button_style_base, LV_OPA_TRANSP);
    lv_style_set_bg_color(&button_style_base, lv_color_white());
    lv_style_set_border_width(&button_style_base, 2);
    lv_style_set_border_opa(&button_style_base, LV_OPA_40);
<<<<<<< HEAD
    lv_style_set_border_color(&button_style_base, lv_color_white());//lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_outline_opa(&button_style_base, LV_OPA_COVER);
    lv_style_set_outline_color(&button_style_base, lv_color_white());

    // Button Style Pressed - lv_config.h - #define LV_THEME_DEFAULT_GROW 0
    static lv_style_t button_style_press;
    lv_style_init(&button_style_press);
    lv_style_set_outline_width(&button_style_press, 0);
    lv_style_set_outline_opa(&button_style_press, LV_OPA_TRANSP);
    lv_style_set_bg_opa(&button_style_press, LV_OPA_50);

    // Left Button
    lv_obj_t *button_left = lv_btn_create(tile_picowalker);  
    lv_obj_align(button_left, LV_ALIGN_CENTER, -60, LR_BUTTON_Y_OFFSET);
    lv_obj_set_size(button_left, 30, 30);
    lv_group_add_obj(tile_group, button_left);
    lv_obj_set_ext_click_area(button_left, 10);
    lv_obj_add_style(button_left, &button_style_base, 0);
    lv_obj_add_style(button_left, &button_style_press, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button_left, button_left_callback, LV_EVENT_CLICKED, NULL);

    // Middle Button
    lv_obj_t *button_middle = lv_btn_create(tile_picowalker);
    lv_obj_align(button_middle, LV_ALIGN_CENTER, 0, MD_BUTTON_Y_OFFSET);
    lv_obj_set_size(button_middle, 37, 37);
    lv_group_add_obj(tile_group, button_middle);
    lv_obj_set_ext_click_area(button_middle, 10);
    lv_obj_add_style(button_middle, &button_style_base, 0);
    lv_obj_add_style(button_middle, &button_style_press, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button_middle, button_middle_callback, LV_EVENT_CLICKED, NULL);

    // Right Button
    lv_obj_t *button_right = lv_btn_create(tile_picowalker);
    lv_obj_align(button_right, LV_ALIGN_CENTER, 60, LR_BUTTON_Y_OFFSET);
    lv_obj_set_size(button_right, 30, 30);
    lv_group_add_obj(tile_group, button_right);
    lv_obj_set_ext_click_area(button_right, 10);
    lv_obj_add_style(button_right, &button_style_base, 0);
    lv_obj_add_style(button_right, &button_style_press, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button_right, button_right_callback, LV_EVENT_CLICKED, NULL);

    // Picowalker Canvas (clickable for step simulation)
    canvas = lv_canvas_create(tile_picowalker);
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, CANVAS_Y_OFFSET);
    lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(canvas, canvas_press_callback, LV_EVENT_PRESSED, NULL);
=======
    lv_style_set_border_color(&button_style_base, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_outline_opa(&button_style_base, LV_OPA_COVER);
    lv_style_set_outline_color(&button_style_base, lv_color_white());

    // Button Style Pressed
    static lv_style_t button_style_press;
    lv_style_init(&button_style_press);
    lv_style_set_translate_y(&button_style_press, 5);
    lv_style_set_outline_width(&button_style_press, 5);
    lv_style_set_outline_opa(&button_style_press, LV_OPA_TRANSP);
    lv_style_set_bg_color(&button_style_press, lv_palette_main(LV_PALETTE_GREY));

    // Left Button
    lv_obj_t *button_left = lv_btn_create(tile_picowalker);     
    lv_obj_set_size(button_left, 30, 30);                
    lv_obj_align(button_left, LV_ALIGN_CENTER, -60, 70);
    lv_group_add_obj(tile_group, button_left);
    lv_obj_add_style(button_left,&button_style_base, 0);
    lv_obj_add_style(button_left,&button_style_press,LV_STATE_CHECKED);
    lv_obj_add_event_cb(button_left, button_left_callback, LV_EVENT_CLICKED, NULL);

    // Middle Button
    lv_obj_t *button_middle = lv_btn_create(tile_picowalker);     
    lv_obj_set_size(button_middle, 37, 37);                
    lv_obj_align(button_middle, LV_ALIGN_CENTER, 0, 80);
    lv_group_add_obj(tile_group, button_middle);
    lv_obj_add_style(button_middle,&button_style_base, 0);
    lv_obj_add_style(button_middle,&button_style_press,LV_STATE_CHECKED);
    lv_obj_add_event_cb(button_middle, button_middle_callback, LV_EVENT_CLICKED, NULL);

    // Right Button
    lv_obj_t *button_right = lv_btn_create(tile_picowalker);     
    lv_obj_set_size(button_right, 30, 30);                
    lv_obj_align(button_right, LV_ALIGN_CENTER, 60, 70);
    lv_group_add_obj(tile_group, button_right);
    lv_obj_add_style(button_right,&button_style_base, 0);
    lv_obj_add_style(button_right,&button_style_press,LV_STATE_CHECKED);
    lv_obj_add_event_cb(button_right, button_right_callback, LV_EVENT_CLICKED, NULL);

    // Picowalker Canvas (no styling)
    canvas = lv_canvas_create(tile_picowalker);
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, -10);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    lv_canvas_fill_bg(canvas, lv_color_make(195, 205, 185), LV_OPA_COVER);
    
    // Rounded overlay to create rounded corners effect
    lv_obj_t *canvas_overlay = lv_obj_create(tile_picowalker);
    lv_obj_set_size(canvas_overlay, CANVAS_WIDTH + 10, CANVAS_HEIGHT + 10);
<<<<<<< HEAD
    lv_obj_align(canvas_overlay, LV_ALIGN_CENTER, 0, CANVAS_Y_OFFSET);
=======
    lv_obj_align(canvas_overlay, LV_ALIGN_CENTER, 0, -10);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    lv_obj_set_style_radius(canvas_overlay, 10, 0);
    lv_obj_set_style_border_width(canvas_overlay, 5, 0);
    lv_obj_set_style_border_color(canvas_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(canvas_overlay, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(canvas_overlay, LV_OBJ_FLAG_CLICKABLE);

    // System Menu Tile
<<<<<<< HEAD
    lv_obj_t *tile_menu = lv_tileview_add_tile(tile_view, 0, 1, LV_DIR_TOP|LV_DIR_BOTTOM);
    lv_obj_t *tile_menu_label = lv_label_create(tile_menu);
    lv_label_set_text(tile_menu_label, "System Menu");
    lv_obj_align(tile_menu_label, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_text_color(tile_menu_label, lv_color_black(), 0);

    //  Slider Style
    static lv_style_t slider_style_base;
    lv_style_init(&slider_style_base);
=======
    lv_obj_t *tile_menu = lv_tileview_add_tile(tile_view, 0, 1, LV_DIR_TOP);

    //  Slider Style
    static lv_style_t slider_style_base;
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    lv_style_set_bg_color(&slider_style_base, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_border_color(&slider_style_base, lv_palette_darken(LV_PALETTE_ORANGE, 3));

    // Slider Style Indicator
    static lv_style_t slider_style_indictator;
    lv_style_init(&slider_style_indictator);
    lv_style_set_bg_color(&slider_style_indictator, lv_palette_lighten(LV_PALETTE_DEEP_ORANGE, 3));
    lv_style_set_bg_grad_color(&slider_style_indictator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_grad_dir(&slider_style_indictator, LV_GRAD_DIR_HOR);

    // Slider Style Press
    static lv_style_t slider_style_indictator_press;
    lv_style_init(&slider_style_indictator_press);
    lv_style_set_shadow_color(&slider_style_indictator_press, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_shadow_width(&slider_style_indictator_press, 10);
    lv_style_set_shadow_spread(&slider_style_indictator_press, 3);

    // Brightness Slider
<<<<<<< HEAD
    brightness_slider = lv_slider_create(tile_menu);
    lv_obj_set_size(brightness_slider, 120, 10);
=======
    lv_obj_t *brightness_slider = lv_slider_create(tile_menu);
    lv_obj_set_size(brightness_slider, 150, 10);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    lv_obj_align(brightness_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(brightness_slider, 0, 100);
    lv_slider_set_value(brightness_slider, 10, LV_ANIM_OFF);    // TODO review a saved state in eeprom.
    lv_obj_add_style(brightness_slider, &slider_style_base,0);
    lv_obj_add_style(brightness_slider, &slider_style_indictator,LV_PART_INDICATOR);
    lv_obj_add_style(brightness_slider, &slider_style_indictator_press, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(brightness_slider, &slider_style_base,LV_PART_KNOB);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Label for Brightness Slider
<<<<<<< HEAD
    brightness_label = lv_label_create(tile_menu); 
    static char initial_brightness_text[32];
    snprintf(initial_brightness_text, sizeof(initial_brightness_text), "Brightness: %d%%", (int)lv_slider_get_value(brightness_slider));
    lv_label_set_text(brightness_label, initial_brightness_text);
    lv_obj_align(brightness_label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_text_color(brightness_label, lv_color_black(), 0);
    lv_group_add_obj(tile_group, brightness_slider);

    // Battery Bar Style
    static lv_style_t battery_style_base;
    lv_style_init(&battery_style_base);
    lv_style_set_bg_color(&battery_style_base, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_border_color(&battery_style_base, lv_palette_darken(LV_PALETTE_GREY, 2));
    lv_style_set_border_width(&battery_style_base, 1);
    lv_style_set_radius(&battery_style_base, 3);
    
    // Battery Bar Indicator Style
    static lv_style_t battery_style_indicator;
    lv_style_init(&battery_style_indicator);
    lv_style_set_bg_color(&battery_style_indicator, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_bg_grad_color(&battery_style_indicator, lv_palette_lighten(LV_PALETTE_GREEN, 2));
    lv_style_set_bg_grad_dir(&battery_style_indicator, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&battery_style_indicator, 3);
    
    // Battery Bar
    battery_bar = lv_bar_create(tile_menu);
    lv_obj_set_size(battery_bar, 120, 15);
    lv_obj_align(battery_bar, LV_ALIGN_CENTER, 0, 40);
    lv_bar_set_range(battery_bar, 0, 100);
    lv_obj_add_style(battery_bar, &battery_style_base, 0);
    lv_obj_add_style(battery_bar, &battery_style_indicator, LV_PART_INDICATOR);
    
    // Battery Label
    battery_label = lv_label_create(tile_menu);
    lv_label_set_text(battery_label, "Battery: 0%");
    lv_obj_align(battery_label, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(battery_label, lv_color_black(), 0);
    
    // Start battery update timer (update every 30 seconds)
    add_repeating_timer_ms(30000, repeating_battery_timer_callback, NULL, &battery_timer);
    
    // Initial battery reading
    repeating_battery_timer_callback(NULL);

    // EEPROM Menu Tile
    lv_obj_t *tile_eeprom = lv_tileview_add_tile(tile_view, 0, 2, LV_DIR_TOP);
    lv_obj_t *tile_eeprom_label = lv_label_create(tile_eeprom);
    lv_label_set_text(tile_eeprom_label, "EEPROM Menu");
    lv_obj_align(tile_eeprom_label, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_text_color(tile_eeprom_label, lv_color_black(), 0);

    // EEPROM Save Button
    lv_obj_t *eeprom_save_button = lv_btn_create(tile_eeprom);
    lv_obj_set_size(eeprom_save_button, 120, 30);
    lv_obj_align(eeprom_save_button, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(eeprom_save_button, eeprom_save_button_callback, LV_EVENT_PRESSED, NULL);
    
    // EEPROM Save Button Label
    lv_obj_t *eeprom_save_label = lv_label_create(eeprom_save_button);
    lv_label_set_text(eeprom_save_label, "Save EEPROM");
    lv_obj_center(eeprom_save_label);
    lv_obj_set_style_text_font(eeprom_save_label, &lv_font_montserrat_14, 0);
    lv_group_add_obj(tile_group, eeprom_save_button);

    // EEPROM Wipe Button
    lv_obj_t *eeprom_wipe_button = lv_btn_create(tile_eeprom);
    lv_obj_set_size(eeprom_wipe_button, 120, 30);
    lv_obj_align(eeprom_wipe_button, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(eeprom_wipe_button, eeprom_wipe_button_callback, LV_EVENT_LONG_PRESSED, NULL);
    
    // Set button background color to red
    lv_obj_set_style_bg_color(eeprom_wipe_button, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_bg_color(eeprom_wipe_button, lv_palette_darken(LV_PALETTE_RED, 2), LV_STATE_PRESSED);
    
    // EEPROM Wipe Button Label
    lv_obj_t *eeprom_wipe_label = lv_label_create(eeprom_wipe_button);
    lv_label_set_text(eeprom_wipe_label, "Wipe EEPROM");
    lv_obj_center(eeprom_wipe_label);
    lv_obj_set_style_text_font(eeprom_wipe_label, &lv_font_montserrat_14, 0);
    lv_group_add_obj(tile_group, eeprom_wipe_button);
=======
    lv_obj_t *label = lv_label_create(brightness_slider);
    lv_label_set_text(label, "Brightness");
    lv_obj_center(label);
    lv_group_add_obj(tile_group, brightness_slider);

    // Battery Bar
    lv_obj_t *battery_bar = lv_bar_create(tile_menu);
    lv_obj_set_size(battery_bar, 150, 10);
    lv_obj_align(battery_bar, LV_ALIGN_CENTER, 0, 30);
    lv_bar_set_range(battery_bar, 0, 100);

    pw_battery_status_t battery_status = pw_power_get_battery_status();
    lv_bar_set_value(battery_bar, battery_status.percent, LV_ANIM_OFF);

    lv_obj_t *battery_label = lv_label_create(battery_bar);
    lv_label_set_text_fmt(battery_label, "Battery: %d%%", battery_status.percent);
    lv_obj_center(battery_label);
    lv_group_add_obj(tile_group, battery_bar);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)

}

/********************************************************************************
 * @brief           Gets Color (Greyscale)
 * @param color     Color from the screen_colour_t (Greyscale)
 * @return lv_color_t
********************************************************************************/
lv_color_t get_color(screen_colour_t color)
{
    lv_color_t lv_color;
    
    // Convert PW colors to LVGL colors
    switch(color) 
    {
        case SCREEN_WHITE: lv_color = lv_color_make(195, 205, 185); break;
        case SCREEN_LGREY: lv_color = lv_color_make(170,170,170); break;
        case SCREEN_DGREY: lv_color = lv_color_make(85,85,85); break;
        case SCREEN_BLACK: lv_color = lv_color_black(); break;
        default: lv_color = lv_color_white(); break;
    }
    
    return lv_color;
}

/********************************************************************************
<<<<<<< HEAD
=======
 * @brief           Get pixel color from packed image data
 * @param image     Image structure
 * @param col       Column position in image
 * @param row       Row position in image
 * @return screen_colour_t
********************************************************************************/
screen_colour_t get_pixel_from_image(pw_img_t *image, int col, int row)
{
    // Each byte contains 4 pixels (2 bits each)
    size_t pixel_index = row * image->width + col;
    size_t byte_index = pixel_index / 4;
    size_t bit_offset = (pixel_index % 4) * 2;
    return (image->data[byte_index] >> bit_offset) & 0x03;
}

/********************************************************************************
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
 * @brief           Draws scaled area to canvas
 * @param x         Screen position X
 * @param y         Screen position Y  
 * @param width     Width of area to draw
 * @param height    Height of area to draw
 * @param color     LVGL color to fill area
********************************************************************************/
void draw_to_scale(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height, lv_color_t color)
<<<<<<< HEAD
{
=======
{    
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    // Width and height of area to draw
    for (screen_pos_t row = 0; row < height; row++) 
    {
        for (screen_pos_t col = 0; col < width; col++) 
        {
            // Draw scaled pixel using fractional scaling
<<<<<<< HEAD
            int start_x = (x + col) * CANVAS_SCALE;
            int start_y = (y + row) * CANVAS_SCALE;
            int end_x = (x + col + 1) * CANVAS_SCALE;
            int end_y = (y + row + 1) * CANVAS_SCALE;
=======
            int start_x = ((x + col) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            int start_y = ((y + row) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            int end_x = ((x + col + 1) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            int end_y = ((y + row + 1) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
            
            // Fill the scaled pixel area
            for (int sy = start_y; sy < end_y && sy < CANVAS_HEIGHT; sy++) {
                for (int sx = start_x; sx < end_x && sx < CANVAS_WIDTH; sx++) {
                    lv_canvas_set_px(canvas, sx, sy, color);
                }
            }
        }
    }
}
<<<<<<< HEAD

=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
/********************************************************************************
 * @brief           Draws image to Canvas
 * @param image     Incoming image of picowalker
 * @param x         Screen position x
 * @param y         Screen position y
********************************************************************************/
void pw_screen_draw_img(pw_img_t *image, screen_pos_t x, screen_pos_t y)
{
    if (!canvas || !image || !image->data) return;

    // Calculate image size (2 bytes per 8 pixels)
<<<<<<< HEAD
    image->size = image->width * image->height * 2 / 8;

    // Process image data in chunks of 2 bytes (8 pixels each)
    for (size_t i = 0; i < image->size; i += 2)
    {
        uint8_t bpp_upper = image->data[i + 0];
        uint8_t bpp_lower = image->data[i + 1];

        // Process 8 pixels from this byte pair
        for (size_t j = 0; j < 8; j++)
        {
            // Extract 2-bit pixel value (same as working code)
            uint8_t pixel_value = ((bpp_upper >> j) & 1) << 1;
            pixel_value |= ((bpp_lower >> j) & 1);

=======
    size_t img_size = image->width * image->height * 2 / 8;
    
    // Process image data in chunks of 2 bytes (8 pixels each)
    for (size_t i = 0; i < img_size; i += 2) {
        uint8_t bpu = image->data[i + 0];  // Upper bits
        uint8_t bpl = image->data[i + 1];  // Lower bits
        
        // Process 8 pixels from this byte pair
        for (size_t j = 0; j < 8; j++) {
            // Extract 2-bit pixel value
            screen_colour_t pixel_value = ((bpu >> j) & 1) << 1;
            pixel_value |= ((bpl >> j) & 1);
            
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
            // Calculate pixel coordinates
            size_t x_normal = (i / 2) % image->width;
            size_t y_normal = 8 * (i / (2 * image->width)) + j;
            
            // Skip if pixel is outside image bounds
<<<<<<< HEAD
            //if (x_normal >= image->width || y_normal >= image->height) continue;

            lv_color_t lv_color = get_color(pixel_value);

            for (size_t py = 0; py < CANVAS_SCALE; py++)
            {
                for (size_t px = 0; px < CANVAS_SCALE; px++)
                {
                    int canvas_x = (x + x_normal) * CANVAS_SCALE + px; 
                    int canvas_y = (y + y_normal) * CANVAS_SCALE + py;
                    lv_canvas_set_px(canvas, canvas_x, canvas_y, lv_color);
=======
            if (x_normal >= image->width || y_normal >= image->height) continue;
            
            // Convert to LVGL color
            lv_color_t lv_color = get_color(pixel_value);
            
            // Draw scaled pixel using fractional scaling
            int start_x = ((x + x_normal) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            int start_y = ((y + y_normal) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            int end_x = ((x + x_normal + 1) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            int end_y = ((y + y_normal + 1) * PW_SCALE_NUMERATOR) / PW_SCALE_DENOMINATOR;
            
            // Fill the scaled pixel area
            for (int sy = start_y; sy < end_y && sy < CANVAS_HEIGHT; sy++) {
                for (int sx = start_x; sx < end_x && sx < CANVAS_WIDTH; sx++) {
                    lv_canvas_set_px(canvas, sx, sy, lv_color);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
                }
            }
        }
    }
}

/********************************************************************************
 * @brief           Clears area on canvas to background color
 * @param x         Screen position X
 * @param y         Screen position Y  
 * @param width     Width of area to clear
 * @param height    Height of area to clear
********************************************************************************/
void pw_screen_clear_area(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height)
{
    if (!canvas) return;
    
    // Clear area by setting pixels directly to background color with scaling
    lv_color_t bg_color = lv_color_make(195, 205, 185);
    draw_to_scale(x, y, width, height, bg_color);
}

/********************************************************************************
 * @brief           Draws a Horizontal Line
 * @param x         Screen position X
 * @param y         Screen position Y
 * @param width     Width of the line
 * @param color     Color of pixel
********************************************************************************/
void pw_screen_draw_horiz_line(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_colour_t color)
{
    if (!canvas) return;

    lv_color_t lv_color = get_color(color);
    draw_to_scale(x, y, width, 1, lv_color);
}

/********************************************************************************
 * @brief           Draws a Rectangle
 * @param x         Screen position X
 * @param y         Screen position Y
 * @param width     Width of rectangle
 * @param height    Height of rectangle
 * @param color     Color of pixel
********************************************************************************/
void pw_screen_draw_text_box(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height, screen_colour_t color)
{
    if (!canvas) return;
    
    // Convert PW color to LVGL color
    lv_color_t lv_color = get_color(color);
    
    // Draw the 4 border lines using draw_to_scale()
    // Top horizontal line
    draw_to_scale(x, y, width, 1, lv_color);
    
    // Bottom horizontal line  
    draw_to_scale(x, y + height - 1, width, 1, lv_color);
    
    // Left vertical line
    draw_to_scale(x, y, 1, height, lv_color);
    
    // Right vertical line
    draw_to_scale(x + width - 1, y, 1, height, lv_color);
}

/********************************************************************************
 * @brief           Clears the Canvas
 * @param N/A
********************************************************************************/
void pw_screen_clear()
{
    if (!canvas) return;
    lv_canvas_fill_bg(canvas, lv_color_make(195, 205, 185), LV_OPA_COVER);
}

/********************************************************************************
 * @brief           Fills Area of Pixel Color
 * @param x         Screen position x
 * @param y         Screen position y
 * @param width     Width of the area
 * @param height    Height of the area
 * @param color     Color of pixel
********************************************************************************/
void pw_screen_fill_area(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height, screen_colour_t color)
{
    if (!canvas) return;
    
    lv_color_t lv_color = get_color(color);
    draw_to_scale(x, y, width, height, lv_color);
}

/********************************************************************************
 * @brief           Puts screen into sleep mode
 * @param N/A
********************************************************************************/
void pw_screen_sleep()
{
    WS_Digital_Write(LCD_DC_PIN, 0);       // Command mode
    WS_SPI_WriteByte(LCD_SPI_PORT, 0x10);  // SLPIN (Sleep In) command
    WS_SET_PWM(0);                         // Turn off backlight

<<<<<<< HEAD
    // Stop LVGL processing and battery updates
    cancel_repeating_timer(&lvgl_timer);
    cancel_repeating_timer(&battery_timer);
=======
    // Stop LVGL processing
    cancel_repeating_timer(&lvgl_timer);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
    is_sleeping = true;
}

/********************************************************************************
 * @brief           Wakes screen from sleep mode
 * @param N/A
********************************************************************************/
void pw_screen_wake()
{
    WS_Digital_Write(LCD_DC_PIN, 0);       // Command mode
    WS_SPI_WriteByte(LCD_SPI_PORT, 0x11);  // SLPOUT (Sleep Out) command
    sleep_ms(120);                          // Wait for LCD to wake
    WS_SET_PWM(10);                        // Restore backlight

<<<<<<< HEAD
    // Restart LVGL processing and battery updates
    add_repeating_timer_ms(5, repeating_lvgl_timer_callback, NULL, &lvgl_timer);
    add_repeating_timer_ms(30000, repeating_battery_timer_callback, NULL, &battery_timer);
    is_sleeping = false;
    
    // Update battery immediately on wake
    repeating_battery_timer_callback(NULL);
=======
    // Restart LVGL processing
    add_repeating_timer_ms(5, repeating_lvgl_timer_callback, NULL, &lvgl_timer);
    is_sleeping = false;
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
}