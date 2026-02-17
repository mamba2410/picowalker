#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "st7789v2_rp2350_lvgl.h"
#include "adc_rp2xxx.h"
#include "qmi8658_rp2xxx.h"

#include "picowalker_rp2350_color_icons.h"
#include "picowalker_rp2350_color_routes.h"
#include "picowalker_rp2350_color_pokemon_small.h"
#include "picowalker_rp2350_color_pokemon_large.h"
#include "picowalker_rp2350_color_pokemon_large_shiny.h"
#include "picowalker_rp2350_color_rle.h"

#include "emulated_rp2xxx.h"

#ifdef __has_include
#if __has_include("board_resources.h")
#include "board_resources.h"
#endif
#endif

// LVGL Settings
static lv_disp_drv_t driver_display;
static lv_disp_draw_buf_t display_buffer;
static lv_disp_t default_display;
static lv_draw_ctx_t draw_ctx;


#define LVGL_BUFFER_DIVISOR 2
static lv_color_t buffer0[240 * 320 / LVGL_BUFFER_DIVISOR];
static lv_color_t buffer1[240 * 320 / LVGL_BUFFER_DIVISOR];
#if CANVAS_SCALE >= 3
// For CANVAS_SCALE >= 3: Use full-size image buffer without zoom
static lv_color_t image_buffer[CANVAS_WIDTH * CANVAS_HEIGHT]; // 288x192 = 110KB but no zoom blur
static lv_obj_t *image_obj; // Image object to display the buffer
static lv_img_dsc_t image_dsc; // Image descriptor
#else
static lv_color_t canvas_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];
#endif

static lv_indev_drv_t driver_touch;
static uint16_t touch_x;
static uint16_t touch_y;
static lv_indev_state_t touch_state;

static lv_group_t *tile_group;
static lv_obj_t *canvas;

static lv_obj_t *brightness_slider;
static lv_obj_t *brightness_label;

static lv_obj_t *battery_bar;
static lv_obj_t *battery_label;
static lv_obj_t *tile_view;

static struct repeating_timer lvgl_timer;
static struct repeating_timer battery_timer;
bool is_sleeping = false;

// Array of background images
const lv_img_dsc_t *backgrounds[] = {
    &picowalker_background,
    &picowalker_background_ultra,
    // Add more backgrounds here
};

#define NUM_BACKGROUNDS (sizeof(backgrounds) / sizeof(backgrounds[0]))

// Current background index
static uint8_t background_index = 0;

// Your background image object (create this once at startup)
lv_obj_t *background_image;

// Swipe event callback
static void background_callback(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *object = lv_event_get_target(event);
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t index = lv_dropdown_get_selected(object);
        lv_img_set_src(background_image, backgrounds[index]);
    }
}

// Cache metadata for Pokemon Walk End
typedef struct {
    uint16_t species;
    uint8_t pokemon_flags_1;
    uint8_t pokemon_flags_2;
} metadata_t;

metadata_t metadata;

/********************************************************************************
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
    
    for(uint32_t i = 0; i < pixel_count; i++) 
    {
        uint16_t pixel = pixel_data[i];
        pixel_data[i] = (pixel << 8) | (pixel >> 8);
    }

    ST7789V2_Set_Windows(area->x1, area->y1, area->x2 , area->y2);
    dma_channel_configure(ST7789V2_DMA_TX,
                          &ST7789V2_DMA_CONFIG,
                          &spi_get_hw(SCREEN_SPI_PORT)->dr, 
                          color,
                          ((area->x2 + 1 - area-> x1)*(area->y2 + 1 - area -> y1))*2,
                          //pixel_count * 2,
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
        CST816S_Get_Point(SCREEN_ROTATION, ST7789V2_WIDTH, ST7789V2_HEIGHT);
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
    pw_button_callback(PW_BUTTON_L);
}

/********************************************************************************
 * @brief           Middle Button Callback
 * @param event     LVGL event call back
********************************************************************************/
static void button_middle_callback(lv_event_t *event)
{
    pw_button_callback(PW_BUTTON_M);
}

/********************************************************************************
 * @brief           Right Button Callback
 * @param event     LVGL event call back
********************************************************************************/
static void button_right_callback(lv_event_t *event)
{
    pw_button_callback(PW_BUTTON_R);
}

/********************************************************************************
 * @brief           Canvas Press Callback - adds steps when canvas is pressed
 * @param event     LVGL event from canvas
********************************************************************************/
static void button_steps_callback(lv_event_t * event)
{
    pw_accel_add_steps(1000);
    printf("[Debug] Steps pressed - step added!\n");
}

/********************************************************************************
 * @brief           Direct Memory Access Handler for Display
 * @param N/A
********************************************************************************/
static void direct_memory_access_handler(void)
{
    if (dma_channel_get_irq0_status(ST7789V2_DMA_TX)) 
    {
        dma_channel_acknowledge_irq0(ST7789V2_DMA_TX);
        lv_disp_flush_ready(&driver_display); // Indicate you are ready with the flushing
    }
}

/********************************************************************************
 * @brief           Changes brightness of the screen
 * @param event     LVGL event from indev input
********************************************************************************/
static void brightness_slider_event_callback(lv_event_t *event)
{
      lv_obj_t *slider = lv_event_get_target(event);
      int32_t value = lv_slider_get_value(slider);
      ST7789V2_Set_PWM(value);
      
      static char brightness_text[32];
      snprintf(brightness_text, sizeof(brightness_text), "Brightness: %d%%", (int)value);
      lv_label_set_text(brightness_label, brightness_text);
}

/********************************************************************************
 * @brief           EEPROM Wipe Button Callback
 * @param event     LVGL event from button LONG press
********************************************************************************/
static void eeprom_reset_button_callback(lv_event_t * event)
{
    if (event->code == LV_EVENT_LONG_PRESSED) 
    {
        printf("[EEPROM] Wiping EEPROM via button LONG press...\n");
        
        // Wipe entire EEPROM to erased state (0xFF)
        // pw_eeprom_set_area(0, 0xFF, 64 * 1024);
        // printf("[EEPROM] EEPROM wiped successfully!\n");
        // Reset steps counter as well since EEPROM is wiped
        // pw_accel_reset_steps();
        pw_eeprom_reset(true, true);
        printf("[EEPROM] EEPROM reset successfully!\n");
    }
}

/********************************************************************************
 * @brief           EEPROM Save Button Callback
 * @param event     LVGL event from button press
********************************************************************************/
static void eeprom_save_button_callback(lv_event_t * event)
{
    if (event->code == LV_EVENT_PRESSED) 
    {
        // play_confirm_sound();
        printf("[EEPROM] Saving EEPROM via button press...\n");
        
        // Check if there are changes to save
        if (pw_eeprom_is_cache_dirty()) 
        {
            if (pw_eeprom_flush_to_flash() == 0) printf("[EEPROM] EEPROM saved successfully!\n");
            else printf("[EEPROM] Failed to save EEPROM!\n");
        } 
        else printf("[EEPROM] No changes to save\n");
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
    // pw_battery_status_t battery_status = pw_power_get_battery_status();
    
    // // Update battery bar value with smooth animation
    // lv_bar_set_value(battery_bar, battery_status.percent, LV_ANIM_ON);
    
    // // Update battery label with percentage
    // if (battery_label) 
    // {
    //     static char battery_text[32];
    //     snprintf(battery_text, sizeof(battery_text), "Battery: %d%%", battery_status.percent);
    //     lv_label_set_text(battery_label, battery_text);
    // }
    
    return true;
}

/********************************************************************************
 * @brief           Tileview Event Callback - triggered when tiles change
 * @param event     LVGL event from tileview
********************************************************************************/
static void tileview_event_callback(lv_event_t * event)
{
    lv_obj_t *tileview = lv_event_get_target(event);
    
    if (event->code == LV_EVENT_SCROLL_END) 
    {
        // Force immediate battery update when tile changes
        repeating_battery_timer_callback(NULL);
        printf("[Debug] Tile changed - battery updated\n");
    }
}

/********************************************************************************
 * @brief           Manual Battery Update (can be called externally)
 * @param N/A
********************************************************************************/
void pw_screen_update_battery()
{
    repeating_battery_timer_callback(NULL);
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
    // Initialize WaveShare 1.69" LCD - Screen
    ST7789V2_Init(SCREEN_ROTATION);
    ST7789V2_Set_PWM(10);
    ST7789V2_Clear(BLACK);

    // Initialize LVGL Display
    lv_disp_draw_buf_init(&display_buffer, buffer0, buffer1, DISP_HOR_RES * DISP_VER_RES / LVGL_BUFFER_DIVISOR); 
    lv_disp_drv_init(&driver_display);    
    driver_display.flush_cb = display_flush_callback;
    driver_display.draw_buf = &display_buffer;        
    driver_display.hor_res = DISP_HOR_RES;
    driver_display.ver_res = DISP_VER_RES;
    lv_disp_t *display = lv_disp_drv_register(&driver_display);
    // lv_disp_set_rotation(display, LV_DISP_ROT_90); // TODO
#if TOUCH
    // Initialize Touch Screen - Button
    CST816S_init(CST816S_Point_Mode);

    // Initialize Touch Screen Input
    lv_indev_drv_init(&driver_touch); 
    driver_touch.type = LV_INDEV_TYPE_POINTER;    
    driver_touch.read_cb = touch_read_callback;            
    lv_indev_t * touch_screen = lv_indev_drv_register(&driver_touch);
    gpio_set_irq_enabled_with_callback(TOUCH_INT_PIN, GPIO_IRQ_EDGE_RISE, true, &touch_callback);
#endif
    // Initialize DMA Direct Memory Access
    dma_channel_set_irq0_enabled(ST7789V2_DMA_TX, true);
    irq_set_exclusive_handler(DMA_IRQ_0, direct_memory_access_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Picowalker Screen
    lv_obj_t *screen = lv_scr_act();

    // Picowalker Tile
    tile_group = lv_group_create();
    tile_view = lv_tileview_create(screen);
    lv_obj_set_scrollbar_mode(tile_view,  LV_SCROLLBAR_MODE_OFF);
    lv_group_add_obj(tile_group, tile_view);
    
    // Add tileview event callback for tile changes
    lv_obj_add_event_cb(tile_view, tileview_event_callback, LV_EVENT_SCROLL_END, NULL);
    lv_obj_t *tile_picowalker = lv_tileview_add_tile(tile_view, 0, 0, LV_DIR_BOTTOM);

    // Set tile background color to match canvas/image background
    if (true) lv_obj_set_style_bg_color(tile_picowalker, lv_color_white(), 0);
    else lv_obj_set_style_bg_color(tile_picowalker, lv_color_make(195, 205, 185), 0);
    lv_obj_set_style_bg_opa(tile_picowalker, LV_OPA_COVER, 0);

#if CANVAS_SCALE <= 2
    // Pokeball Image ... I want to add more
    LV_IMG_DECLARE(picowalker_background);
    LV_IMG_DECLARE(picowalker_background_ultra);
    background_image = lv_img_create(tile_picowalker);
    lv_img_set_src(background_image, backgrounds[0]); //&picowalker_background);
    lv_obj_align(background_image, LV_ALIGN_CENTER, 0, 0);

    // Button Style Not Pressed
    static lv_style_t button_style_base;
    lv_style_init(&button_style_base);
    lv_style_set_radius(&button_style_base, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&button_style_base, LV_OPA_TRANSP);
    lv_style_set_bg_color(&button_style_base, lv_color_white());
    lv_style_set_border_width(&button_style_base, 2);
    lv_style_set_border_opa(&button_style_base, LV_OPA_40);
    lv_style_set_border_color(&button_style_base, lv_color_white());//lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_outline_opa(&button_style_base, LV_OPA_COVER);
    lv_style_set_outline_color(&button_style_base, lv_color_white());

    // Button Style Pressed - lv_config.h - #define LV_THEME_DEFAULT_GROW 0
    static lv_style_t button_style_press;
    lv_style_init(&button_style_press);
    lv_style_set_outline_width(&button_style_press, 0);
    lv_style_set_outline_opa(&button_style_press, LV_OPA_TRANSP);
    lv_style_set_bg_opa(&button_style_press, LV_OPA_50);
#else
    // Debugging
    // static lv_style_t button_style_invisible;
    // lv_style_init(&button_style_invisible);
    // lv_style_set_bg_opa(&button_style_invisible, LV_OPA_30); // Semi-transparent
    // lv_style_set_bg_color(&button_style_invisible, lv_color_make(255, 0, 0)); // Red
    // lv_style_set_border_width(&button_style_invisible, 2);
    // lv_style_set_border_color(&button_style_invisible, lv_color_make(255, 255, 0)); // Yellow border
    // lv_style_set_border_opa(&button_style_invisible, LV_OPA_COVER);

#endif
    // Invisible button style for CANVAS_SCALE >= 3
    static lv_style_t button_style_invisible;
    lv_style_init(&button_style_invisible);
    lv_style_set_bg_opa(&button_style_invisible, LV_OPA_TRANSP);
    lv_style_set_border_opa(&button_style_invisible, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&button_style_invisible, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&button_style_invisible, LV_OPA_TRANSP);

#if CANVAS_SCALE <= 2
    // Left Button
    lv_obj_t *button_left = lv_btn_create(tile_picowalker);
    lv_obj_align(button_left, LV_ALIGN_CENTER, -60, LR_BUTTON_Y_OFFSET);
    lv_obj_set_size(button_left, 30, 30);
    lv_group_add_obj(tile_group, button_left);
    lv_obj_set_ext_click_area(button_left, 15);
    lv_obj_add_style(button_left, &button_style_base, 0);
    lv_obj_add_style(button_left, &button_style_press, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button_left, button_left_callback, LV_EVENT_CLICKED, NULL);

    // Middle Button
    lv_obj_t *button_middle = lv_btn_create(tile_picowalker);
    lv_obj_align(button_middle, LV_ALIGN_CENTER, 0, MD_BUTTON_Y_OFFSET);
    lv_obj_set_size(button_middle, 37, 37);
    lv_group_add_obj(tile_group, button_middle);
    lv_obj_set_ext_click_area(button_middle, 15);
    lv_obj_add_style(button_middle, &button_style_base, 0);
    lv_obj_add_style(button_middle, &button_style_press, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button_middle, button_middle_callback, LV_EVENT_CLICKED, NULL);

    // Right Button
    lv_obj_t *button_right = lv_btn_create(tile_picowalker);
    lv_obj_align(button_right, LV_ALIGN_CENTER, 60, LR_BUTTON_Y_OFFSET);
    lv_obj_set_size(button_right, 30, 30);
    lv_group_add_obj(tile_group, button_right);
    lv_obj_set_ext_click_area(button_right, 15);
    lv_obj_add_style(button_right, &button_style_base, 0);
    lv_obj_add_style(button_right, &button_style_press, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button_right, button_right_callback, LV_EVENT_CLICKED, NULL);

    // Top Button (for adding steps)
    lv_obj_t *button_top = lv_btn_create(tile_picowalker);
    lv_obj_align(button_top, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(button_top, 37, 37);
    lv_group_add_obj(tile_group, button_top);
    lv_obj_set_ext_click_area(button_right, 15);
    lv_obj_add_style(button_top, &button_style_invisible, 0);
    lv_obj_add_event_cb(button_top, button_steps_callback, LV_EVENT_CLICKED, NULL);

#else
    // CANVAS_SCALE >= 3: Invisible full-screen buttons taking 1/3 of screen each
    // Left Button (bottom left third)
    lv_obj_t *button_left = lv_btn_create(tile_picowalker);
    lv_obj_set_size(button_left, DISP_HOR_RES / 3, DISP_VER_RES * 2 / 3);
    lv_obj_align(button_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_style(button_left, &button_style_invisible, 0);
    lv_obj_add_event_cb(button_left, button_left_callback, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(tile_group, button_left);

    // Middle Button (bottom middle third)
    lv_obj_t *button_middle = lv_btn_create(tile_picowalker);
    lv_obj_set_size(button_middle, DISP_HOR_RES / 3, DISP_VER_RES * 2 / 3);
    lv_obj_align(button_middle, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(button_middle, &button_style_invisible, 0);
    lv_obj_add_event_cb(button_middle, button_middle_callback, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(tile_group, button_middle);

    // Right Button (bottom right third)
    lv_obj_t *button_right = lv_btn_create(tile_picowalker);
    lv_obj_set_size(button_right, DISP_HOR_RES / 3, DISP_VER_RES * 2 / 3);
    lv_obj_align(button_right, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_add_style(button_right, &button_style_invisible, 0);
    lv_obj_add_event_cb(button_right, button_right_callback, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(tile_group, button_right);

    // Top Button (for adding steps)
    lv_obj_t *button_top = lv_btn_create(tile_picowalker);
    lv_obj_set_size(button_top, DISP_HOR_RES, DISP_VER_RES / 3);
    lv_obj_align(button_top, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(button_top, &button_style_invisible, 0);
    lv_obj_add_event_cb(button_top, button_steps_callback, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(tile_group, button_top);
#endif

#if CANVAS_SCALE <= 2
    // Picowalker Canvas (clickable for step simulation)
    canvas = lv_canvas_create(tile_picowalker);
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, CANVAS_Y_OFFSET);
    lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(canvas, button_steps_callback, LV_EVENT_PRESSED, NULL);
    if (true) lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER); // is_color = true
    else lv_canvas_fill_bg(canvas, lv_color_make(195, 205, 185), LV_OPA_COVER);

    // Rounded overlay to create rounded corners effect
    lv_obj_t *canvas_overlay = lv_obj_create(tile_picowalker);
    lv_obj_set_size(canvas_overlay, CANVAS_WIDTH + 10, CANVAS_HEIGHT + 10);
    lv_obj_align(canvas_overlay, LV_ALIGN_CENTER, 0, CANVAS_Y_OFFSET);
    lv_obj_set_style_radius(canvas_overlay, 10, 0);
    lv_obj_set_style_border_width(canvas_overlay, 5, 0);
    lv_obj_set_style_border_color(canvas_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(canvas_overlay, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(canvas_overlay, LV_OBJ_FLAG_CLICKABLE);
#else
    // For CANVAS_SCALE >= 3: Create image object at full scaled resolution
    // Initialize image descriptor with our full-size buffer
    image_dsc.header.always_zero = 0;
    image_dsc.header.w = CANVAS_WIDTH;
    image_dsc.header.h = CANVAS_HEIGHT;
    image_dsc.data_size = CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(lv_color_t);
    image_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    image_dsc.data = (uint8_t*)image_buffer;

    // Clear image buffer to background color
    lv_color_t bg_color;
    if (true) bg_color = lv_color_white();
    else bg_color = lv_color_make(195, 205, 185);

    for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) {
        image_buffer[i] = bg_color;
    }

    // Create image object at full resolution (no zoom)
    image_obj = lv_img_create(tile_picowalker);
    lv_img_set_src(image_obj, &image_dsc);
    lv_obj_set_size(image_obj, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_align(image_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor_opa(image_obj, LV_OPA_TRANSP, 0);

    canvas = NULL; // No canvas in this mode
#endif

    // System Menu Tile
    lv_obj_t *tile_menu = lv_tileview_add_tile(tile_view, 0, 1, LV_DIR_TOP|LV_DIR_BOTTOM);
    lv_obj_t *tile_menu_label = lv_label_create(tile_menu);
    lv_label_set_text(tile_menu_label, "System Menu");
    lv_obj_align(tile_menu_label, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_text_color(tile_menu_label, lv_color_black(), 0);

    // Drowpdown Background
    lv_obj_t *dropdown = lv_dropdown_create(tile_menu);
    lv_dropdown_set_options(dropdown, "Pokeball\n" "Ultra");
    lv_obj_align(dropdown, LV_ALIGN_CENTER, 0, -40);
    lv_obj_add_event_cb(dropdown, background_callback, LV_EVENT_ALL, NULL);

    //  Slider Style
    static lv_style_t slider_style_base;
    lv_style_init(&slider_style_base);
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
    brightness_slider = lv_slider_create(tile_menu);
    lv_obj_set_size(brightness_slider, 120, 10);
    lv_obj_align(brightness_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(brightness_slider, 0, 100);
    lv_slider_set_value(brightness_slider, 10, LV_ANIM_OFF);    // TODO review a saved state in eeprom.
    lv_obj_add_style(brightness_slider, &slider_style_base,0);
    lv_obj_add_style(brightness_slider, &slider_style_indictator,LV_PART_INDICATOR);
    lv_obj_add_style(brightness_slider, &slider_style_indictator_press, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(brightness_slider, &slider_style_base,LV_PART_KNOB);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Label for Brightness Slider
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
    lv_obj_t *eeprom_reset_button = lv_btn_create(tile_eeprom);
    lv_obj_set_size(eeprom_reset_button, 120, 30);
    lv_obj_align(eeprom_reset_button, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(eeprom_reset_button, eeprom_reset_button_callback, LV_EVENT_LONG_PRESSED, NULL);
    
    // Set button background color to red
    lv_obj_set_style_bg_color(eeprom_reset_button, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_bg_color(eeprom_reset_button, lv_palette_darken(LV_PALETTE_RED, 2), LV_STATE_PRESSED);
    
    // EEPROM reset Button Label
    lv_obj_t *eeprom_reset_label = lv_label_create(eeprom_reset_button);
    lv_label_set_text(eeprom_reset_label, "reset EEPROM");
    lv_obj_center(eeprom_reset_label);
    lv_obj_set_style_text_font(eeprom_reset_label, &lv_font_montserrat_14, 0);
    lv_group_add_obj(tile_group, eeprom_reset_button);
}

/********************************************************************************
 * @brief           Gets Color from pixel value or RGB565
 * @param color     Color value (0-3 for greyscale enum, or >3 for RGB565)
 * @return lv_color_t
********************************************************************************/
lv_color_t get_color(uint16_t color, bool is_color)
{
    lv_color_t lv_color;

    // Convert PW greyscale colors to LVGL colors
    if (is_color)
    {
        switch(color)
        {
            case PW_SCREEN_WHITE: lv_color = lv_color_white(); break; //lv_color_make(195, 205, 185); break;
            case PW_SCREEN_LGREY: lv_color = lv_color_make(135, 135, 161); break; //lv_color_make(170,170,170); break;
            case PW_SCREEN_DGREY: lv_color = lv_color_make(88, 88, 138); break; //lv_color_make(85,85,85); break;
            case PW_SCREEN_BLACK: lv_color = lv_color_black(); break;
            default: 
                // RGB565 color - extract RGB components
                uint8_t r = ((color >> 11) & 0x1F) << 3; // 5-bit red -> 8-bit
                uint8_t g = ((color >> 5) & 0x3F) << 2;  // 6-bit green -> 8-bit
                uint8_t b = (color & 0x1F) << 3;         // 5-bit blue -> 8-bit

                // Expand to full 8-bit range
                r |= (r >> 5);
                g |= (g >> 6);
                b |= (b >> 5);

                lv_color = lv_color_make(r, g, b);
                break;
        }
    }
    else
    {
        switch(color)
        {
            case PW_SCREEN_WHITE: lv_color = lv_color_make(195, 205, 185); break;
            case PW_SCREEN_LGREY: lv_color = lv_color_make(170,170,170); break;
            case PW_SCREEN_DGREY: lv_color = lv_color_make(85,85,85); break;
            case PW_SCREEN_BLACK: lv_color = lv_color_black(); break;
            default: lv_color = lv_color_white(); break;
        }
    }


    return lv_color;
}

/********************************************************************************
 * @brief           Draws scaled area to canvas
 * @param x         Screen position X
 * @param y         Screen position Y  
 * @param width     Width of area to draw
 * @param height    Height of area to draw
 * @param color     LVGL color to fill area
********************************************************************************/
void draw_to_scale(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t width, pw_screen_pos_t height, lv_color_t color)
{
    // Width and height of area to draw
    for (pw_screen_pos_t row = 0; row < height; row++) 
    {
        for (pw_screen_pos_t col = 0; col < width; col++) 
        {
            // Draw scaled pixel using fractional scaling
            int start_x = (x + col) * CANVAS_SCALE;
            int start_y = (y + row) * CANVAS_SCALE;
            int end_x = (x + col + 1) * CANVAS_SCALE;
            int end_y = (y + row + 1) * CANVAS_SCALE;

            // Fill the scaled pixel area
            for (int sy = start_y; sy < end_y && sy < CANVAS_HEIGHT; sy++)
            {
                for (int sx = start_x; sx < end_x && sx < CANVAS_WIDTH; sx++)
                {
#if CANVAS_SCALE <= 2
                    lv_canvas_set_px(canvas, sx, sy, color);
#else
                    image_buffer[sy * CANVAS_WIDTH + sx] = color;
#endif
                }
            }
        }
    }

#if CANVAS_SCALE >= 3
    // Invalidate image to trigger LVGL redraw with scaling
    lv_obj_invalidate(image_obj);
#endif
}

// /*
//  *  size: 16 bytes
//  *  dmitry: struct PokemonSummary
//  */
// typedef struct {
//     /* +0x00 */ uint16_t le_species;
//     /* +0x02 */ uint16_t le_held_item;
//     /* +0x04 */ uint16_t le_moves[4];
//     /* +0x0c */ uint8_t level;
//     /* +0x0d */ struct {
//         uint8_t variant : 5;      // bits 0-4: variant index (0-31)
//         uint8_t unused : 1;       // bit 5: unused
//         uint8_t is_female : 1;    // bit 6: gender flag
//         uint8_t unused2 : 1;      // bit 7: unused
//     } flags_1;
//     /* +0x0e */ struct {
//         uint8_t has_form : 1;     // bit 0: has form flag
//         uint8_t is_shiny : 1;     // bit 1: shiny flag
//         uint8_t unused : 6;       // bits 2-7: unused
//     } flags_2;
//     /* +0x0f */ uint8_t padding;
// } pokemon_summary_t;

/********************************************************************************
 * @brief           Draws image to Canvas
 * @param image     Incoming image of picowalker
 * @param x         Screen position x
 * @param y         Screen position y
********************************************************************************/
void pw_screen_draw_img(pw_img_t *image, pw_screen_pos_t x, pw_screen_pos_t y)
{
#if CANVAS_SCALE <= 2
    if (!canvas || !image || !image->data) return;
#else
    if (!image_obj || !image || !image->data) return;
#endif

    uint8_t *image_data = image->data;
    uint8_t *bin_data;
    uint32_t uncompressed_size;
    bool is_color = false;

    // Our Pokemon
    bool is_transparent = false;
    bool is_second_frame = false;
    uint16_t species;
    uint8_t pokemon_flags_1;
    uint8_t pokemon_flags_2;
    uint8_t variant;            // pokemon_flags_1 & 0x1F: AND mask extracts bits 0-4 (variant index: 0-31)
    bool is_female;             // pokemon_flags_1 & 0x20: AND mask extracts bit 5 (gender: 0x00 or 0x20)
    bool has_form;              // pokemon_flags_2 & 0x01: AND mask extracts bit 0 (has_form flag)
    bool is_shiny;              // pokemon_flags_2 & 0x02: AND mask extracts bit 1 (shiny flag)


// Check if we should use alternate color lookup (RGB565)
    if (image->lookup_table.use_alt)
    {
        // Our Pokemon
        if (image->lookup_table.addr == 0x933E || image->lookup_table.addr == 0x963E // Large
        || image->lookup_table.addr == 0x91BE || image->lookup_table.addr == 0x927E) // Small
        {
            // uint16_t species;
            // uint8_t pokemon_flags_1;
            // uint8_t pokemon_flags_2;
            pw_eeprom_read(0x8F00, (uint8_t*)&species, 2);
            pw_eeprom_read(0x8F0D, &pokemon_flags_1, 1);
            pw_eeprom_read(0x8F0E, &pokemon_flags_2, 1);
            
            // Pokemon Walk Start & End Animation for Small and Large respectively...
            if (species == 0 & pokemon_flags_1 == 0 & pokemon_flags_2 == 0) 
            {
                species = metadata.species;
                pokemon_flags_1 = metadata.pokemon_flags_1;
                pokemon_flags_2 = metadata.pokemon_flags_2;
            }
            else 
            {
                metadata.species = species;
                metadata.pokemon_flags_1 = pokemon_flags_1;
                metadata.pokemon_flags_2 = pokemon_flags_2;
            }
            
        }
            
        if (image->lookup_table.addr == 0x933E || image->lookup_table.addr == 0x963E) // Large
        {
            uint8_t variant = pokemon_flags_1 & 0x1F;
            bool is_female = pokemon_flags_1 & 0x20;
            bool has_form = pokemon_flags_2 & 0x01;
            bool is_shiny = pokemon_flags_2 & 0x02;
            if (has_form) is_female = 0; // variants assume the male form.
            
            const pokemon_large_entry_t *poke_large;
            // printf("[COLOR_POKEMON_LARGE] Species: %u Variant: %u Female: %u Form:%u Shiny:%u\n", species, variant, is_female, has_form, is_shiny); 
            if (is_shiny) poke_large = find_pokemon_large_shiny(species, variant, is_female);
            else poke_large = find_pokemon_large(species, variant, is_female);

            if (poke_large != NULL)
            {
                if (is_shiny) bin_data = color_pokemon_large_shiny + poke_large->bin_offset;
                else bin_data = color_pokemon_large + poke_large->bin_offset;

                image->width = 64;
                image->height = 48;
                is_color = true;
                is_transparent = true;
                uncompressed_size = poke_large->uncompressed_size;

                if (image->lookup_table.addr == 0x963E) is_second_frame = true;
            }
        }
        // Pokemon Small, All of them...
        if (image->lookup_table.addr == 0x91BE || image->lookup_table.addr == 0x927E    // 0 Pokemon (Ours)
        || image->lookup_table.addr == 0x9D7E || image->lookup_table.addr == 0x9E3E     // 1 Pokemon
        || image->lookup_table.addr == 0x9A7E || image->lookup_table.addr == 0x9B3E     // 2 Pokemon
        || image->lookup_table.addr == 0x9BFE || image->lookup_table.addr == 0x9CBE)    // 3 Pokemon
        {   
            species = image->lookup_table.metadata.pokemon.species;
            variant = image->lookup_table.metadata.pokemon.pokemon_flags_1 & 0x1F;

            // No Lookup Table Metadata Pokemon Walk Start for Small...
            if (species == 0 & variant == 0) 
            {
                species = metadata.species;
                variant = metadata.pokemon_flags_1 & 0x1F;
            }

            printf("[COLOR_POKEMON_SMALL] Species: %u Variant: %u\n", species, variant); 
            const pokemon_small_entry_t *poke_small = find_pokemon_small(species, variant);

            if (poke_small != NULL)
            {
                bin_data = color_pokemon_small + poke_small->bin_offset;
                uncompressed_size = poke_small->uncompressed_size;
                image->width = 32;
                image->height = 24;
                is_color = true;
                is_transparent = true;

                // Second Frame
                if (image->lookup_table.addr == 0x927E 
                    || image->lookup_table.addr == 0x9E3E
                    || image->lookup_table.addr == 0x9B3E
                    || image->lookup_table.addr == 0x9CBE)
                {
                    is_second_frame = true;
                }
            }
        }
        // Routes
        else if (image->lookup_table.addr == 0x8FBE)
        {   
            uint8_t index;
            pw_eeprom_read(0x8F27, &index, 1); // 0x8F27 Route Index
            const color_routes_t *route = find_route_by_index(index);

            if (route != NULL)
            {
                bin_data = color_routes + route->bin_offset;
                uncompressed_size = route->uncompressed_size;
                image->width = 32;
                image->height = 24;
                is_color = true;
            }
        }
        // Icons
        else
        {
            const color_icons_t *icons = find_icon_by_eeprom_address(image->lookup_table.addr);
            
            if (icons != NULL)
            {
                bin_data = color_icons + icons->bin_offset;
                uncompressed_size = icons->uncompressed_size;
                image->width = icons->width;
                image->height = icons->height;
                is_color = true;
            }
        }
    }

    // Calculate image size (2 bytes per 8 pixels for 2bpp, or 2 bytes per pixel for RGB565)
    if (is_color)
    {
        // RGB565 mode: 2 bytes per pixel
        size_t pixel_count = image->width * image->height;
        // uncompress the image data...
        uint16_t *color_data = (uint16_t *)rle_decompress_rgb565(bin_data, uncompressed_size);
        if (is_second_frame) color_data += (image->width * image->height);
        // uint16_t *color_data = (uint16_t *)image_data;
        uint16_t transparency_color = color_data[0];
        lv_color_t background_color = lv_color_white(); // TODO I need a color pallete...

        for (size_t i = 0; i < pixel_count; i++)
        {
            // Calculate pixel coordinates
            size_t x_normal = i % image->width;
            size_t y_normal = i / image->width;

            lv_color_t lv_color = get_color(color_data[i], true);

            for (size_t py = 0; py < CANVAS_SCALE; py++)
            {
                for (size_t px = 0; px < CANVAS_SCALE; px++)
                {
                    int canvas_x = (x + x_normal) * CANVAS_SCALE + px;
                    int canvas_y = (y + y_normal) * CANVAS_SCALE + py;
                    // lv_canvas_set_px(canvas, canvas_x, canvas_y, lv_color);
#if CANVAS_SCALE <= 2
                    lv_canvas_set_px(canvas, canvas_x, canvas_y, lv_color);
#else
                    if (canvas_x < CANVAS_WIDTH && canvas_y < CANVAS_HEIGHT) {
                        image_buffer[canvas_y * CANVAS_WIDTH + canvas_x] = lv_color;
                    }
#endif
                }
            }
        }
    }
    else
    {
        // Calculate image size (2 bytes per 8 pixels)
        image->size = image->width * image->height * 2 / 8;

        // Process image data in chunks of 2 bytes (8 pixels each)
        for (size_t i = 0; i < image->size; i += 2)
        {
            uint8_t bpp_upper = image->data[i + 0];
            uint8_t bpp_lower = image->data[i + 1];

            // Process 8 pixels from this byte pair
            for (size_t j = 0; j < 8; j++)
            {
                // Extract 2-bit pixel value
                uint8_t pixel_value = ((bpp_upper >> j) & 1) << 1;
                pixel_value |= ((bpp_lower >> j) & 1);

                // Calculate pixel coordinates
                size_t x_normal = (i / 2) % image->width;
                size_t y_normal = 8 * (i / (2 * image->width)) + j;

                lv_color_t lv_color = get_color(pixel_value, true);

                // Draw scaled pixel (both modes use same loop structure)
                for (size_t py = 0; py < CANVAS_SCALE; py++)
                {
                    for (size_t px = 0; px < CANVAS_SCALE; px++)
                    {
                        int scaled_x = (x + x_normal) * CANVAS_SCALE + px;
                        int scaled_y = (y + y_normal) * CANVAS_SCALE + py;
    #if CANVAS_SCALE <= 2
                        lv_canvas_set_px(canvas, scaled_x, scaled_y, lv_color);
    #else
                        if (scaled_x < CANVAS_WIDTH && scaled_y < CANVAS_HEIGHT) {
                            image_buffer[scaled_y * CANVAS_WIDTH + scaled_x] = lv_color;
                        }
    #endif
                    }
                }
            }
        }
    }
#if CANVAS_SCALE >= 3
    // Invalidate image to trigger LVGL redraw with scaling
    lv_obj_invalidate(image_obj);
#endif
}

/********************************************************************************
 * @brief           Clears area on canvas to background color
 * @param x         Screen position X
 * @param y         Screen position Y  
 * @param width     Width of area to clear
 * @param height    Height of area to clear
********************************************************************************/
void pw_screen_clear_area(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t width, pw_screen_pos_t height)
{
#if CANVAS_SCALE <= 2
    if (!canvas) return;
#endif

    // Clear area by setting pixels directly to background color with scaling
    lv_color_t bg_color;
    if (true) bg_color = lv_color_white();
    else bg_color = lv_color_make(195, 205, 185);
    draw_to_scale(x, y, width, height, bg_color);
}

/********************************************************************************
 * @brief           Draws a Horizontal Line
 * @param x         Screen position X
 * @param y         Screen position Y
 * @param width     Width of the line
 * @param color     Color of pixel
********************************************************************************/
void pw_screen_draw_horiz_line(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t width, pw_screen_color_t color)
{
#if CANVAS_SCALE <= 2
    if (!canvas) return;
#endif

    lv_color_t lv_color = get_color(color, true);
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
void pw_screen_draw_text_box(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t width, pw_screen_pos_t height, pw_screen_color_t color)
{
#if CANVAS_SCALE <= 2
    if (!canvas) return;
#endif
    
    // Convert PW color to LVGL color
    lv_color_t lv_color = get_color(color, true);
    
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
#if CANVAS_SCALE <= 2
    if (!canvas) return;
    if (true) lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER); // is_color = true
    else lv_canvas_fill_bg(canvas, lv_color_make(195, 205, 185), LV_OPA_COVER);
#else
    // For CANVAS_SCALE >= 3, clear the full-size image buffer
    if (!image_obj) return;
    
    lv_color_t bg_color;
    if (true) bg_color = lv_color_white();
    else bg_color = lv_color_make(195, 205, 185);

    for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) {
        image_buffer[i] = bg_color;
    }
    lv_obj_invalidate(image_obj);
#endif
}

/********************************************************************************
 * @brief           Fills Area of Pixel Color
 * @param x         Screen position x
 * @param y         Screen position y
 * @param width     Width of the area
 * @param height    Height of the area
 * @param color     Color of pixel
********************************************************************************/
void pw_screen_fill_area(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t width, pw_screen_pos_t height, pw_screen_color_t color)
{
#if CANVAS_SCALE <= 2
    if (!canvas) return;
#endif
    
    lv_color_t lv_color = get_color(color, true);
    draw_to_scale(x, y, width, height, lv_color);
}

/********************************************************************************
 * @brief           Puts screen into sleep mode
 * @param N/A
********************************************************************************/
void pw_screen_sleep()
{
    ST7789V2_Set_PWM(0);
    ST7789V2_Sleep();

    // Stop LVGL processing and battery updates
    cancel_repeating_timer(&lvgl_timer);
    cancel_repeating_timer(&battery_timer);
    is_sleeping = true;
}

/********************************************************************************
 * @brief           Wakes screen from sleep mode
 * @param N/A
********************************************************************************/
void pw_screen_wake()
{
    ST7789V2_Awake();
    ST7789V2_Set_PWM(0);
    sleep_ms(120);
    ST7789V2_Set_PWM(10);

    // Restart LVGL processing and battery updates
    add_repeating_timer_ms(5, repeating_lvgl_timer_callback, NULL, &lvgl_timer);
    add_repeating_timer_ms(30000, repeating_battery_timer_callback, NULL, &battery_timer);
    is_sleeping = false;
    
    // Update battery immediately on wake
    repeating_battery_timer_callback(NULL);
}

/********************************************************************************
 * @brief           Changes brightness of the screen
 * @param event     LVGL event from indev input
********************************************************************************/
void pw_screen_set_brightness(uint8_t level)
{
    if(level > PW_SCREEN_MAX_BRIGHTNESS) return;

    uint8_t brightness_range = ST7789V2_MAX_BRIGHTNESS - ST7789V2_MIN_BRIGHTNESS + 1;
    uint8_t level_range = PW_SCREEN_MAX_BRIGHTNESS + 1;
    float step = (float)brightness_range / (float)level_range;
    uint8_t screen_setting = ST7789V2_MIN_BRIGHTNESS + (uint8_t)((float)level * step);
    ST7789V2_Set_PWM(screen_setting);
    printf("[Debug] Set brightness to 0x%02x (%d)\n", screen_setting, level);
}