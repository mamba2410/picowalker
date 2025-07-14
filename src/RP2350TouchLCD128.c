#include "RP2350TouchLCD128.h"

#include "lib/lvgl/src/core/lv_obj.h"
#include "lib/lvgl/src/misc/lv_area.h"

// LVGL Display
lv_display_t *display;

// LVGL Buffers
#define BUFFER_PIXELS (DISP_HOR_RES * DISP_VER_RES)
static lv_color_t buffer0[BUFFER_PIXELS];
static lv_color_t buffer1[BUFFER_PIXELS];

#define CANVAS_WIDTH 144  // Original 96, 1.5 x 96 = 144
#define CANVAS_HEIGHT 96  // Original 64, 1.5 x 64 = 96
static lv_color_t canvas_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];

// Touch variables
static uint16_t touch_screen_x;
static uint16_t touch_screen_y;
static lv_indev_state_t touch_state;

// Accel variables
static int16_t difference;
static lv_indev_state_t accel_state;

// Timing
static lv_timer_t *tick_timer;

static void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *colour);
static void touch_callback(uint gpio, uint32_t events);
static void touch_read_callback(lv_indev_t *driver, lv_indev_data_t *data);
static void accel_read_callback(lv_indev_t *driver, lv_indev_data_t *data);
static void direct_memory_access_handler(void);
static bool tick_timer_callback(lv_timer_t *timer);

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

/********************************************************************************
function:	Refresh image by transferring the color data to the SPI bus by DMA
parameter:
********************************************************************************/
static void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *colour)
{

    LCD_1IN28_SetWindows(area->x1, area->y1, area->x2 , area->y2);
    dma_channel_configure(dma_tx,
                          &c,
                          &spi_get_hw(LCD_SPI_PORT)->dr, 
                          colour, // read address
                          ((area->x2 + 1 - area-> x1)*(area->y2 + 1 - area -> y1))*2,
                          true);

}

/********************************************************************************
function:   Touch interrupt handler
parameter:
********************************************************************************/
static void touch_callback(uint gpio, uint32_t events)
{
    if (gpio == Touch_INT_PIN)
    {
        CST816S_Get_Point();
        touch_screen_x = Touch_CTS816.x_point;
        touch_screen_y = Touch_CTS816.y_point;
        touch_state = LV_INDEV_STATE_PRESSED;
    }
}

/********************************************************************************
function:   Update touch screen input device status
parameter:
********************************************************************************/
static void touch_read_callback(lv_indev_t *driver, lv_indev_data_t *data)
{
    data->point.x = touch_screen_x;
    data->point.y = touch_screen_y; 
    data->state = touch_state;
    touch_state = LV_INDEV_STATE_RELEASED;
}

/********************************************************************************
function:	Update encoder input device status
parameter:
********************************************************************************/
static void accel_read_callback(lv_indev_t *driver, lv_indev_data_t *data)
{
    //TODO Figure out how I am going to get readings...
    //data->enc_diff = encoder_diff;
    //data->state    = encoder_act; 
    /* encoder_diff = 0; */
}

/********************************************************************************
function:   Report the elapsed time to LVGL each 5ms
parameter:
********************************************************************************/
static bool tick_timer_callback(lv_timer_t *t) 
{
    lv_tick_inc(5);
    return true;
}

/********************************************************************************
function:   Indicate ready with the flushing when DMA complete transmission
parameter:
********************************************************************************/
static void direct_memory_access_handler(void)
{
    if (dma_channel_get_irq0_status(dma_tx)) {
        dma_channel_acknowledge_irq0(dma_tx);
        lv_display_flush_ready(display);         /* Indicate you are ready with the flushing*/
    }
}

int RP2350TouchLCD128PW(void)
{
    // Must initialize SPI and GPIO before display init
    if (DEV_Module_Init() != 0) {
        printf("DEV_Module_Init failed.\n");
        return -1;
    }
    // Initialize WaveShare 1.28" LCD - Screen
    DEV_SET_PWM(100);
    LCD_1IN28_Init(HORIZONTAL);
    LCD_1IN28_Clear(WHITE);


    // Initialize Touch Screen - Button
    CST816S_init(CST816S_Point_Mode);
    printf("Touch screen initialized\n");

    // Initialize IMU - Accel
    QMI8658_init();
    printf("IMU initialized\n");

    // Initialize LVGL
    lv_init();
    printf("LVGL initialized\n");

    // Initialize LVGL Display
    lv_display_t *display = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    lv_display_set_flush_cb(display, display_flush_callback);
    lv_display_set_buffers(display, buffer0, buffer1, sizeof(buffer0), LV_DISPLAY_RENDER_MODE_FULL);
    printf("LVGL display configured\n");

    // Iniitialize Touch Screen Input
    lv_indev_t *touch = lv_indev_create();
    lv_indev_set_type(touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch, touch_read_callback);

    // Initialize Accelerometer Input
    lv_indev_t *accel = lv_indev_create();
    lv_indev_set_type(accel, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(accel, accel_read_callback);
    gpio_set_irq_enabled_with_callback(Touch_INT_PIN, GPIO_IRQ_EDGE_FALL, true, &touch_callback);

    // Group Input Device
    lv_group_t *group = lv_group_create();
    lv_indev_set_group(touch, group);
    lv_indev_set_group(accel, group);

    // Initialize DMA Direct Memory Access
    dma_channel_set_irq0_enabled(dma_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, direct_memory_access_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    lv_obj_t *screen = lv_screen_active();

    /* Create the canvas widget */
    //LV_DRAW_BUF_DEFINE_STATIC(canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_RGB565);
    //LV_DRAW_BUF_INIT_STATIC(canvas_buffer);
    //lv_obj_t *canvas = lv_canvas_create(screen);
    //lv_canvas_set_draw_buf(canvas, &canvas_buffer);
    //lv_obj_align(canvas, LV_ALIGN_CENTER, 0, -10);
    //lv_obj_clear_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    //lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER); //lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);
    //lv_obj_set_style_border_width(canvas, 2, 0);
    //lv_obj_set_style_border_color(canvas, lv_color_black(), 0);
    //lv_layer_t layer;
    //lv_canvas_init_layer(canvas, &layer);
    //lv_canvas_finish_layer(canvas, &layer);
    
    /* Your round buttons below the canvas */
    lv_obj_t *left_button = lv_btn_create(screen);
    lv_obj_add_event_cb(left_button, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(left_button, LV_ALIGN_CENTER, -60, 70);
    lv_obj_set_size(left_button, 30, 30);
    lv_obj_set_style_radius(left_button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(left_button, lv_color_white(), 0);
    lv_obj_set_style_border_width(left_button, 2, 0);
    lv_obj_set_style_border_color(left_button, lv_color_black(), 0);

    lv_obj_t *center_button = lv_btn_create(screen);
    lv_obj_add_event_cb(center_button, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(center_button, LV_ALIGN_CENTER, 0, 80);
    lv_obj_set_size(center_button, 37, 37);
    lv_obj_set_style_radius(center_button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_button, lv_color_white(), 0);
    lv_obj_set_style_border_width(center_button, 2, 0);
    lv_obj_set_style_border_color(center_button, lv_color_black(), 0);

    lv_obj_t *right_button = lv_btn_create(screen);
    lv_obj_add_event_cb(right_button, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(right_button, LV_ALIGN_CENTER, 60, 70);
    lv_obj_set_size(right_button, 30, 30);
    lv_obj_set_style_radius(right_button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(right_button, lv_color_white(), 0);
    lv_obj_set_style_border_width(right_button, 2, 0);
    lv_obj_set_style_border_color(right_button, lv_color_black(), 0);

    /*Change the active screen's background color*/
    //lv_obj_t *screen = lv_screen_active();
    //lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    //lv_obj_set_style_bg_opa(screen, LV_OPA_100, 0);


    /*Create a white label, set its text and align it to the center*/
    //lv_obj_t *label = lv_label_create(screen);
    //lv_obj_set_align(label, LV_ALIGN_CENTER);
    //lv_obj_set_height(label, LV_SIZE_CONTENT);
    //lv_obj_set_width(label, LV_SIZE_CONTENT);
    //lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    //lv_obj_set_style_text_color(label, lv_color_black(), 0);
    //lv_label_set_text(label, "Hello World!");
    //printf("Label created\n");

    tick_timer = lv_timer_create(tick_timer_callback, 5, NULL);

    printf("LVGL Initialized Successfully!\r\n");
    return 0;
}