#include "rp2350touchlcd128lvgl.h"
<<<<<<< HEAD
#include <stdio.h>

// LVGL
static lv_disp_drv_t driver_display;
static lv_disp_draw_buf_t display_buffer;
#ifdef PICO_RP2350
#define LVGL_BUFFER_DIVISOR 2
static lv_color_t buffer0[DISP_HOR_RES * DISP_VER_RES/LVGL_BUFFER_DIVISOR];
static lv_color_t buffer1[DISP_HOR_RES * DISP_VER_RES/LVGL_BUFFER_DIVISOR];
#else
#define LVGL_BUFFER_DIVISOR 2
static lv_color_t *buffer0;
static lv_color_t *buffer1;
#endif


#define CANVAS_WIDTH  144  // Original 96, 1.5 x 96 = 144, 2 x 96 = 192
#define CANVAS_HEIGHT 96   // Original 64, 1.5 x 64 = 96,  2 x 64 = 128
static lv_obj_t *canvas;
static lv_color_t canvas_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];
static lv_obj_t *brightness_label;  // Global reference to brightness label
=======

// LVGL
static lv_disp_drv_t driver_display;
static lv_disp_draw_buf_t display_buffer;
#ifdef PICO_RP2350
#define LVGL_BUFFER_DIVISOR 2
static lv_color_t buffer0[DISP_HOR_RES * DISP_VER_RES/LVGL_BUFFER_DIVISOR];
static lv_color_t buffer1[DISP_HOR_RES * DISP_VER_RES/LVGL_BUFFER_DIVISOR];
#else
#define LVGL_BUFFER_DIVISOR 2
static lv_color_t *buffer0;
static lv_color_t *buffer1;
#endif


#define CANVAS_WIDTH  144  // Original 96, 1.5 x 96 = 144, 2 x 96 = 192
#define CANVAS_HEIGHT 96   // Original 64, 1.5 x 64 = 96,  2 x 64 = 128
<<<<<<< HEAD
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
static lv_obj_t *canvas;
static lv_color_t canvas_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)

static lv_indev_drv_t driver_touch;
static lv_indev_drv_t driver_accel;
static lv_group_t *group;

static lv_obj_t *label_imu;

// Input Device 
static int16_t accel_difference;
static lv_indev_state_t accel_state;

static uint16_t touch_x;
static uint16_t touch_y;
static lv_indev_state_t touch_state;

// Timer 
static struct repeating_timer lvgl_timer;
static struct repeating_timer update_timer;
static struct repeating_timer difference_timer;


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
static bool repeating_update_timer_callback(struct repeating_timer *timer)
{
    //char label_text[64];
    float acc[3], gyro[3];
    unsigned int timer_count = 0;
   
    QMI8658_read_xyz(acc, gyro, &timer_count);
    //sprintf(label_text,"%4.1f \n%4.1f \n%4.1f \n\n%4.1f \n%4.1f \n%4.1f ",acc[0],acc[1],acc[2],gyro[0],gyro[1],gyro[2]);
    //lv_label_set_text(label_imu,label_text);
    return true;
}

/********************************************************************************
function:	
parameter:
********************************************************************************/
static bool repeating_difference_timer_callback(struct repeating_timer *timer)
{
    // TODO add difference data
    return true;
}

/********************************************************************************
function:	
parameter:
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
function:	
parameter:
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
function:	
parameter:
********************************************************************************/
static void touch_read_callback(lv_indev_drv_t *driver, lv_indev_data_t *data)
{
    data->point.x = touch_x;
    data->point.y = touch_y; 
    data->state = touch_state;
    touch_state = LV_INDEV_STATE_RELEASED;
}

/********************************************************************************
function:	
parameter:
********************************************************************************/
static void accel_read_callback(lv_indev_drv_t *driver, lv_indev_data_t *data)
{
    data->enc_diff = accel_difference;
    data->state    = accel_state; 
}

/********************************************************************************
function:	
parameter:
********************************************************************************/
static void direct_memory_access_handler(void)
{
    if (dma_channel_get_irq0_status(dma_tx)) {
        dma_channel_acknowledge_irq0(dma_tx);
        lv_disp_flush_ready(&driver_display);         /* Indicate you are ready with the flushing*/
    }
}

/********************************************************************************
function:	
parameter:
********************************************************************************/
static void brightness_slider_event_callback(lv_event_t * event)
{
      lv_obj_t *slider = lv_event_get_target(event);
      int32_t value = lv_slider_get_value(slider);
      WS_SET_PWM(value);
<<<<<<< HEAD
      
      // Update the brightness label to show current value
      static char brightness_text[32];
      snprintf(brightness_text, sizeof(brightness_text), "Brightness: %d%%", (int)value);
      lv_label_set_text(brightness_label, brightness_text);
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
}

/********************************************************************************
function:	
parameter:
********************************************************************************/
int main()
{
    // Must initialize SPI and GPIO before display init
    if (WS_Module_Init() != 0) {
        printf("WS_Module_Init failed.\n");
        return -1;
    }

    // Initialize WaveShare 1.28" LCD - Screen
    WS_SET_PWM(10);
    GC9A01A_Init(HORIZONTAL);
    GC9A01A_Clear(WHITE);

    // Initialize Touch Screen - Button
    CST816S_init(CST816S_Point_Mode);
    printf("Touch screen initialized\n");

    // Initialize IMU - Accel
    QMI8658_init();
    printf("IMU initialized\n");

    // Inital Timers before LVGL
    add_repeating_timer_ms(500, repeating_update_timer_callback,     NULL, &update_timer);
    add_repeating_timer_ms(50,  repeating_difference_timer_callback, NULL, &difference_timer);
    add_repeating_timer_ms(5,   repeating_lvgl_timer_callback,       NULL, &lvgl_timer);

    // Initialize LVGL
    lv_init();
    printf("LVGL initialized\n");

    // Initialize LVGL Display
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    #ifdef PICO_RP2040
    buffer0 = malloc((DISP_HOR_RES * DISP_VER_RES / 2) * sizeof(lv_color_t));
    buffer1 = malloc((DISP_HOR_RES * DISP_VER_RES / 2) * sizeof(lv_color_t));
    #endif
    lv_disp_draw_buf_init(&display_buffer, buffer0, buffer1, DISP_HOR_RES * DISP_VER_RES / LVGL_BUFFER_DIVISOR); 
<<<<<<< HEAD
=======
    lv_disp_draw_buf_init(&display_buffer, buffer0, buffer1, DISP_HOR_RES * DISP_VER_RES / 2); 
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    lv_disp_drv_init(&driver_display);    
    driver_display.flush_cb = display_flush_callback;
    driver_display.draw_buf = &display_buffer;        
    driver_display.hor_res = DISP_HOR_RES;
    driver_display.ver_res = DISP_VER_RES;
    lv_disp_t *display = lv_disp_drv_register(&driver_display);  

    // Initialize Touch Screen Input
    lv_indev_drv_init(&driver_touch); 
    driver_touch.type = LV_INDEV_TYPE_POINTER;    
    driver_touch.read_cb = touch_read_callback;            
    lv_indev_t * touch_screen = lv_indev_drv_register(&driver_touch);
    WS_IRQ_SET(TOUCH_INT_PIN, GPIO_IRQ_EDGE_RISE, &touch_callback);

    // Initialize Accelerometer Input
    lv_indev_drv_init(&driver_accel);   
    driver_accel.type = LV_INDEV_TYPE_ENCODER;  
    driver_accel.read_cb = accel_read_callback;         
    lv_indev_t * accelerometer = lv_indev_drv_register(&driver_accel);

    // Group Input Device
    group = lv_group_create();
    lv_indev_set_group(accelerometer, group);

    // Initialize DMA Direct Memory Access
    dma_channel_set_irq0_enabled(dma_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, direct_memory_access_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Picowalker Code...
    lv_obj_t *screen = lv_scr_act();

    // Picowalker Tile
    lv_obj_t *tile_view = lv_tileview_create(screen);
    lv_obj_set_scrollbar_mode(tile_view,  LV_SCROLLBAR_MODE_OFF);
    lv_group_add_obj(group, tile_view);

<<<<<<< HEAD
<<<<<<< HEAD
    lv_obj_t *tile_picowalker = lv_tileview_add_tile(tile_view, 0, 0, LV_DIR_BOTTOM);
=======
    lv_obj_t *tile_picowalker = lv_tileview_add_tile(tile_view, 0, 0, LV_DIR_TOP);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    lv_obj_t *tile_picowalker = lv_tileview_add_tile(tile_view, 0, 0, LV_DIR_BOTTOM);
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)

    LV_IMG_DECLARE(picowalker_background);
    lv_obj_t *background = lv_img_create(tile_picowalker);
    lv_img_set_src(background, &picowalker_background);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

<<<<<<< HEAD
<<<<<<< HEAD
    // Button Style Not Pressed
=======
    // Button Style Base
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    // Button Style Not Pressed
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    static lv_style_t button_style_base;
    lv_style_init(&button_style_base);
    lv_style_set_radius(&button_style_base, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&button_style_base, LV_OPA_TRANSP);
    lv_style_set_bg_color(&button_style_base, lv_color_white());
<<<<<<< HEAD
<<<<<<< HEAD
    lv_style_set_border_width(&button_style_base, 2);
    lv_style_set_border_opa(&button_style_base, LV_OPA_40);
    lv_style_set_border_color(&button_style_base, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_outline_opa(&button_style_base, LV_OPA_COVER);
    lv_style_set_outline_color(&button_style_base, lv_color_white());

    // Button Style Pressed
    static lv_style_t button_style_press;
    lv_style_init(&button_style_press);
    lv_style_set_translate_y(&button_style_press, 5);
    lv_style_set_outline_width(&button_style_press, 5);
    lv_style_set_outline_opa(&button_style_press, LV_OPA_TRANSP);
=======
    lv_style_set_border_opa(&button_style_base, LV_OPA_40);
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    lv_style_set_border_width(&button_style_base, 2);
    lv_style_set_border_opa(&button_style_base, LV_OPA_40);
    lv_style_set_border_color(&button_style_base, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_outline_opa(&button_style_base, LV_OPA_COVER);
    lv_style_set_outline_color(&button_style_base, lv_color_white());

    // Button Style Pressed
    static lv_style_t button_style_press;
    lv_style_init(&button_style_press);
    lv_style_set_translate_y(&button_style_press, 5);
    lv_style_set_outline_width(&button_style_press, 5);
    lv_style_set_outline_opa(&button_style_press, LV_OPA_TRANSP);
<<<<<<< HEAD
    //lv_style_set_translate_y(&button_style_press, 5);
    //lv_style_set_shadow_offset_y(&button_style_press, 3);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    lv_style_set_bg_color(&button_style_press, lv_palette_main(LV_PALETTE_GREY));

    // Left Button
    lv_obj_t *button_left = lv_btn_create(tile_picowalker);     
    lv_obj_set_size(button_left, 30, 30);                
    lv_obj_align(button_left, LV_ALIGN_CENTER, -60, 70);
    lv_group_add_obj(group, button_left);
    lv_obj_add_style(button_left,&button_style_base, 0);
    lv_obj_add_style(button_left,&button_style_press,LV_STATE_CHECKED);
    
    // lv_obj_t *label_left = lv_label_create(button_left);
    // lv_label_set_text(label_left, "◁");
    // lv_obj_center(label_left);

    // Center Button
    lv_obj_t *button_center = lv_btn_create(tile_picowalker);     
    lv_obj_set_size(button_center, 37, 37);                
    lv_obj_align(button_center, LV_ALIGN_CENTER, 0, 80);
    lv_group_add_obj(group, button_center);
    lv_obj_add_style(button_center,&button_style_base, 0);
    lv_obj_add_style(button_center,&button_style_press,LV_STATE_CHECKED);

    // Right Button
    lv_obj_t *button_right = lv_btn_create(tile_picowalker);     
    lv_obj_set_size(button_right, 30, 30);                
    lv_obj_align(button_right, LV_ALIGN_CENTER, 60, 70);
    lv_group_add_obj(group, button_right);
    lv_obj_add_style(button_right,&button_style_base, 0);
    lv_obj_add_style(button_right,&button_style_press,LV_STATE_CHECKED);

    // lv_obj_t *label_right = lv_label_create(button_right);
    // lv_label_set_text(label_right, "▷");
    // lv_obj_center(label_right);
    
<<<<<<< HEAD
<<<<<<< HEAD
    // Picowalker Canvas (no styling)
    canvas = lv_canvas_create(tile_picowalker);
=======
    // Canvas
    static lv_color_t canvas_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];
    lv_obj_t *canvas = lv_canvas_create(tile_picowalker);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    // Picowalker Canvas (no styling)
    canvas = lv_canvas_create(tile_picowalker);
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, -10);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_canvas_fill_bg(canvas, lv_color_make(195, 205, 185), LV_OPA_COVER);
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    
    // Rounded overlay to create rounded corners effect
    lv_obj_t *canvas_overlay = lv_obj_create(tile_picowalker);
    lv_obj_set_size(canvas_overlay, CANVAS_WIDTH + 10, CANVAS_HEIGHT + 10);
    lv_obj_align(canvas_overlay, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_radius(canvas_overlay, 10, 0);
    lv_obj_set_style_border_width(canvas_overlay, 5, 0);
    lv_obj_set_style_border_color(canvas_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(canvas_overlay, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(canvas_overlay, LV_OBJ_FLAG_CLICKABLE);
<<<<<<< HEAD
=======
    lv_obj_set_style_border_width(canvas, 2, 0);
    lv_obj_set_style_border_color(canvas, lv_color_black(), 0);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)

    // Drawing on canvas in v8 is done directly


    // System Menu Tile
<<<<<<< HEAD
<<<<<<< HEAD
    lv_obj_t *tile_menu = lv_tileview_add_tile(tile_view, 0, 2, LV_DIR_TOP);

    //  Slider Style
=======
    lv_obj_t *tile_menu = lv_tileview_add_tile(tile_view, 0, 2, LV_DIR_BOTTOM);

>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    lv_obj_t *tile_menu = lv_tileview_add_tile(tile_view, 0, 2, LV_DIR_TOP);

    //  Slider Style
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    static lv_style_t slider_style_base;
    lv_style_set_bg_color(&slider_style_base, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_border_color(&slider_style_base, lv_palette_darken(LV_PALETTE_ORANGE, 3));

<<<<<<< HEAD
<<<<<<< HEAD
    // Slider Style Indicator
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    // Slider Style Indicator
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    static lv_style_t slider_style_indictator;
    lv_style_init(&slider_style_indictator);
    lv_style_set_bg_color(&slider_style_indictator, lv_palette_lighten(LV_PALETTE_DEEP_ORANGE, 3));
    lv_style_set_bg_grad_color(&slider_style_indictator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_grad_dir(&slider_style_indictator, LV_GRAD_DIR_HOR);

<<<<<<< HEAD
<<<<<<< HEAD
    // Slider Style Press
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    // Slider Style Press
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    static lv_style_t slider_style_indictator_press;
    lv_style_init(&slider_style_indictator_press);
    lv_style_set_shadow_color(&slider_style_indictator_press, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_shadow_width(&slider_style_indictator_press, 10);
    lv_style_set_shadow_spread(&slider_style_indictator_press, 3);
<<<<<<< HEAD
<<<<<<< HEAD

    
    // Brightness Slider
=======
    
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======

    
    // Brightness Slider
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    lv_obj_t *brightness_slider = lv_slider_create(tile_menu);
    lv_obj_set_size(brightness_slider, 150, 10);
    lv_obj_align(brightness_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(brightness_slider, 0, 100);
    lv_slider_set_value(brightness_slider, 10, LV_ANIM_OFF);    // TODO review a saved state in eeprom.
<<<<<<< HEAD
<<<<<<< HEAD
=======
    
    lv_obj_t *label = lv_label_create(brightness_slider);
    lv_label_set_text(label, "Brightness");
    lv_obj_center(label);
    lv_group_add_obj(group, brightness_slider);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)
    lv_obj_add_style(brightness_slider, &slider_style_base,0);
    lv_obj_add_style(brightness_slider, &slider_style_indictator,LV_PART_INDICATOR);
    lv_obj_add_style(brightness_slider, &slider_style_indictator_press, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(brightness_slider, &slider_style_base,LV_PART_KNOB);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
<<<<<<< HEAD
<<<<<<< HEAD
    
    // Label for Brightness Slider
    brightness_label = lv_label_create(brightness_slider);
    // Set initial label text with current slider value
    static char initial_brightness_text[32];
    snprintf(initial_brightness_text, sizeof(initial_brightness_text), "Brightness: %d%%", (int)lv_slider_get_value(brightness_slider));
    lv_label_set_text(brightness_label, initial_brightness_text);
    lv_obj_center(brightness_label);
    lv_group_add_obj(group, brightness_slider);
=======
    // Add a Callback event for the brightness..
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    
    // Label for Brightness Slider
    lv_obj_t *label = lv_label_create(brightness_slider);
    lv_label_set_text(label, "Brightness");
    lv_obj_center(label);
    lv_group_add_obj(group, brightness_slider);
>>>>>>> 0d881bd (Code Cleanup and RP2040 Oddities)


    while(1)
    {
      lv_task_handler();
      WS_Delay_ms(5); 
    }

    WS_Module_Exit();
    return 0;
}