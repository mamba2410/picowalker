#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <pico/stdlib.h>

#include "hardware/gpio.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/spi.h"

//#include "hardware/structs/hstx_ctrl.h"
//#include "hardware/structs/hstx_fifo.h"

#include "../picowalker-defs.h"
#include "screen_rp2350_gc9a01a.h"

static lcd_t lcd = {0};
static uint8_t lcd_buffer[LCD_BUFFER_SIZE] = {0};
static lcd_attributes_t lcd_attributes = {0};

/*
 * Screen for the Waveshare rp2350 touch LCS 1.28"
 * It's a circular screen with touch controls, but touch isn't implemented here.
 * 240x240 resolution, assume corners are cut off?
 */

/********************************************************************************
Function: Resets LCD
Parameters:
********************************************************************************/
 void lcd_reset() 
 {
    // Reset and clear memory
    gpio_put(LCD_PIN_RST, 0);
    sleep_ms(100);
    gpio_put(LCD_PIN_RST, 1);
    sleep_ms(100);
    gpio_put(LCD_PIN_CS, 0);
    sleep_ms(100);
}

/********************************************************************************
Function: Sends command to LCD
Parameters:
        command : command to be sent
********************************************************************************/
void lcd_send_command(uint8_t command)
{
    // DCX goes high to indicate data being sent
    gpio_put(LCD_PIN_DC, 0);
    spi_write_blocking(LCD_SPI, &command, 1);
}

/********************************************************************************
Function: Sends data to LCD
Parameters:
        data : data to be sent
********************************************************************************/
void lcd_send_data(uint8_t data)
{
    // DCX goes high to indicate data being sent
    gpio_put(LCD_PIN_DC, 1);
    spi_write_blocking(LCD_SPI, &data, 1);
}

/********************************************************************************
Function: Sets the LCD to the specified coordinates
Parameters:
        x_start : x start coordinate
        y_start : y start coordinate
        x_end : x end coordinate
        y_end : y end coordinate
 *********************************************************************************/
void lcd_set_windows(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end) 
{
    // Set X Coordinates
    lcd_send_command(CMD_COL_SET); //0x2A
    lcd_send_data(CMD_NOP);        //0x00
    lcd_send_data(x_start);
    lcd_send_data((x_end-1)>>8);
    lcd_send_data(x_end-1);

    // Set Y Coordinates
    lcd_send_command(CMD_PAGE_SET); //0x2B    
    lcd_send_data(CMD_NOP);         //0x00
    lcd_send_data(y_start);
    lcd_send_data((y_start-1)>>8);
    lcd_send_data(y_end-1);

    lcd_send_command(CMD_WRITE_START); //0x2C

}

/********************************************************************************
Function: Clears the screen
Parameters:
********************************************************************************/
void lcd_clear_screen() 
{
    // Clear Screen by sending all zeroes
    uint16_t i;
    uint16_t colour = 0x0000;
    uint16_t image[LCD_WIDTH*LCD_HEIGHT];
    colour = ((colour<<8)&0xff00)|(colour>>8);

    for(i = 0; i < LCD_WIDTH*LCD_HEIGHT; i++) 
    {
        image[i] = colour;
    }

    lcd_set_windows(0, 0, LCD_WIDTH, LCD_HEIGHT);
    gpio_put(LCD_PIN_DC, 1);
    for (i = 0; i < LCD_WIDTH*LCD_HEIGHT; i++) 
    {
        spi_write_blocking(LCD_SPI, (uint8_t *)&image[i*LCD_WIDTH], LCD_WIDTH*2);
    }
        
}

/********************************************************************************
Function: Set Resolution and Scanning Direction
Parameters:
        scanning_direction: Scanning direction
********************************************************************************/
void lcd_set_attributes(uint8_t scan_direction)
{
    // Set screen scan direction
    lcd_attributes.SCAN_DIRECTION = scan_direction;
    uint8_t memory_access_register = 0x08;

    // Get GRAM and LCD width and height
    if (scan_direction == HORIZONTAL)
    {
        lcd_attributes.HEIGHT = LCD_HEIGHT;
        lcd_attributes.WIDTH = LCD_WIDTH;

        memory_access_register = 0xC8;
    }
    else 
    {
        lcd_attributes.HEIGHT = LCD_WIDTH;
        lcd_attributes.WIDTH = LCD_HEIGHT;
        memory_access_register = 0x68;
    }

    // Set read / write scan direction
    lcd_send_command(CMD_MEMORY_ACCESS);
    lcd_send_data(memory_access_register);

}


/********************************************************************************
Function: Initializes the LCD
Parameters:
*********************************************************************************/
void lcd_init()
{
    // Reset and clear memory
    lcd_reset();

    // Set display orientation
    lcd_set_attributes(HORIZONTAL);    

    // Set Intiialization Sequence
    lcd_send_command(CMD_INTER_REG_ENABLE2);    // 0xEF
	lcd_send_command(0xEB);
	lcd_send_data(0x14); 
	
    lcd_send_command(CMD_INTER_REG_ENALBE1);    // 0xFE			 
	lcd_send_command(CMD_INTER_REG_ENABLE2);    // 0xEF 


    // Not sure what this is doing here...
	lcd_send_command(0xEB);	
	lcd_send_data(0x14); 

	lcd_send_command(0x84);			
	lcd_send_data(0x40); 

	lcd_send_command(0x85);			
	lcd_send_data(0xFF); 

	lcd_send_command(0x86);			
	lcd_send_data(0xFF); 

	lcd_send_command(0x87);			
	lcd_send_data(0xFF);

	lcd_send_command(0x88);			
	lcd_send_data(0x0A);

	lcd_send_command(0x89);
	lcd_send_data(0x21); 

	lcd_send_command(0x8A);			
	lcd_send_data(0x00);

	lcd_send_command(0x8B);			
	lcd_send_data(0x80); 

	lcd_send_command(0x8C);			
	lcd_send_data(0x01); 

	lcd_send_command(0x8D);			
	lcd_send_data(0x01); 

	lcd_send_command(0x8E);			
	lcd_send_data(0xFF); 

	lcd_send_command(0x8F);			
	lcd_send_data(0xFF); 


	lcd_send_command(CMD_DISPLAY_FUNCTION);     // 0xB6
	lcd_send_data(0x00); 
	lcd_send_data(0x20);

	lcd_send_command(CMD_MEMORY_ACCESS);        // 0x36
	lcd_send_data(0x08);

	lcd_send_command(CMD_PIXEL_FORMAT);         // 0x3A			
	lcd_send_data(0x05); 


    // Not sure what this is doing here...
	lcd_send_command(0x90);			
	lcd_send_data(0x08);
	lcd_send_data(0x08);
	lcd_send_data(0x08);
	lcd_send_data(0x08); 

	lcd_send_command(0xBD);			
	lcd_send_data(0x06);
	
	lcd_send_command(0xBC);			
	lcd_send_data(0x00);

	lcd_send_command(0xFF);			
	lcd_send_data(0x60);
	lcd_send_data(0x01);
	lcd_send_data(0x04);


	lcd_send_command(CMD_POWER_CONTROL2);       // 0xC3			
	lcd_send_data(0x13);

	lcd_send_command(CMD_POWER_CONTROL3);       // 0xC4			
	lcd_send_data(0x13);

	lcd_send_command(CMD_POWER_CONTROL4);       // 0xC9			
	lcd_send_data(0x22);


    // Not sure what this is doing here...
	lcd_send_command(0xBE);			
	lcd_send_data(0x11);

	lcd_send_command(0xE1);			
	lcd_send_data(0x10);
	lcd_send_data(0x0E);

	lcd_send_command(0xDF);			
	lcd_send_data(0x21);
	lcd_send_data(0x0C);
	lcd_send_data(0x02);


	lcd_send_command(CMD_SET_GAMMA1);           // 0xF0   
	lcd_send_data(0x45);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	lcd_send_data(0x08);
	lcd_send_data(0x26);
 	lcd_send_data(0x2A);


 	lcd_send_command(CMD_SET_GAMMA2);           // 0xF1    
 	lcd_send_data(0x43);
 	lcd_send_data(0x70);
 	lcd_send_data(0x72);
 	lcd_send_data(0x36);
 	lcd_send_data(0x37); 
 	lcd_send_data(0x6F);


 	lcd_send_command(CMD_SET_GAMMA3);           // 0xF2   
 	lcd_send_data(0x45);
 	lcd_send_data(0x09);
 	lcd_send_data(0x08);
 	lcd_send_data(0x08);
 	lcd_send_data(0x26);
 	lcd_send_data(0x2A);

 	lcd_send_command(CMD_SET_GAMMA4);           // 0xF3   
 	lcd_send_data(0x43);
 	lcd_send_data(0x70);
 	lcd_send_data(0x72);
 	lcd_send_data(0x36);
 	lcd_send_data(0x37);
 	lcd_send_data(0x6F);


    // Not sure what this is doing here...
	lcd_send_command(0xED);	
	lcd_send_data(0x1B); 
	lcd_send_data(0x0B); 

	lcd_send_command(0xAE);			
	lcd_send_data(0x77);
	
	lcd_send_command(0xCD);			
	lcd_send_data(0x63);		


    // Not sure what this is doing here...
	lcd_send_command(0x70);			
	lcd_send_data(0x07);
	lcd_send_data(0x07);
	lcd_send_data(0x04);
	lcd_send_data(0x0E); 
	lcd_send_data(0x0F); 
	lcd_send_data(0x09);
	lcd_send_data(0x07);
	lcd_send_data(0x08);
	lcd_send_data(0x03);


	lcd_send_command(CMD_FRAME_RATE);
	lcd_send_data(0x34);


    // Not sure what this is doing here...
	lcd_send_command(0x62);			
	lcd_send_data(0x18);
	lcd_send_data(0x0D);
	lcd_send_data(0x71);
	lcd_send_data(0xED);
	lcd_send_data(0x70); 
	lcd_send_data(0x70);
	lcd_send_data(0x18);
	lcd_send_data(0x0F);
	lcd_send_data(0x71);
	lcd_send_data(0xEF);
	lcd_send_data(0x70); 
	lcd_send_data(0x70);

	lcd_send_command(0x63);			
	lcd_send_data(0x18);
	lcd_send_data(0x11); 
	lcd_send_data(0x71);
	lcd_send_data(0xF1); 
	lcd_send_data(0x70); 
	lcd_send_data(0x70);
	lcd_send_data(0x18);
	lcd_send_data(0x13); 
	lcd_send_data(0x71);
	lcd_send_data(0xF3);
	lcd_send_data(0x70); 
	lcd_send_data(0x70);

	lcd_send_command(0x64);			
	lcd_send_data(0x28); 
	lcd_send_data(0x29); 
	lcd_send_data(0xF1); 
	lcd_send_data(0x01);
	lcd_send_data(0xF1); 
	lcd_send_data(0x00);
	lcd_send_data(0x07);

	lcd_send_command(0x66);			
	lcd_send_data(0x3C); 
	lcd_send_data(0x00);
	lcd_send_data(0xCD);
	lcd_send_data(0x67);
	lcd_send_data(0x45); 
	lcd_send_data(0x45);
	lcd_send_data(0x10);
	lcd_send_data(0x00); 
	lcd_send_data(0x00);
	lcd_send_data(0x00);

	lcd_send_command(0x67);			
	lcd_send_data(0x00);
	lcd_send_data(0x3C);
	lcd_send_data(0x00);
	lcd_send_data(0x00);
	lcd_send_data(0x00);
	lcd_send_data(0x01);
	lcd_send_data(0x54);
	lcd_send_data(0x10);
	lcd_send_data(0x32);
	lcd_send_data(0x98);

	lcd_send_command(0x74);			
	lcd_send_data(0x10);	
	lcd_send_data(0x85);	
	lcd_send_data(0x80);
	lcd_send_data(0x00); 
	lcd_send_data(0x00); 
	lcd_send_data(0x4E);
	lcd_send_data(0x00);					
	
    lcd_send_command(0x98);			
	lcd_send_data(0x3e);
	lcd_send_data(0x07);


	lcd_send_command(CMD_TEARING_ON);           // 0x35	
	lcd_send_command(CMD_INVERT_ON);            // 0x21

    lcd_send_command(CMD_SLEEP_OUT);            // 0x11
    sleep_ms(3);
    lcd_send_command(CMD_DISPLAY_ON);           // 0x29
    sleep_ms(20);

}


/********************************************************************************
Function: Sends image buffer in RAM to display, ideal for background image
Parameters:
    image: pointer to image buffer
*********************************************************************************/
void lcd_display(uint16_t *image)
{
    uint16_t i;
    lcd_set_windows(0, 0, LCD_WIDTH, LCD_HEIGHT);
    gpio_put(LCD_PIN_DC, 1);
    for (i = 0; i < LCD_HEIGHT; i++)
    {
        spi_write_blocking(LCD_SPI, (uint8_t *)&image[i*LCD_WIDTH], LCD_WIDTH*2);
    }
}

/********************************************************************************
Function: Sends image buffer in RAM to display at a starting point
Parameters:
    image   : pointer to image buffer
    x_start : x coordinate of starting point
    y_start : y coordinate of starting point
    width   : width of image
    height  : height of image
*********************************************************************************/
void lcd_display_windows(uint16_t *image, uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height)
{
    uint32_t address = 0;
    uint16_t i;
    lcd_set_windows(x_start, y_start, width, height);
    gpio_put(LCD_PIN_DC, 1);
    for (i = y_start; i < width; i++)
    {
        address = x_start + (i * LCD_WIDTH);
        spi_write_blocking(LCD_SPI, (uint8_t *)&image[address], (width-x_start)*2);
    }
}

/********************************************************************************
Function: Draws game frame buffer to display
Parameters:
    buffer  : pointer to game frame buffer
    x_start : x coordinate starting point of image
    y_start : y coordinate starting point of image
    width   : width of buffer
    height  : height of buffer
*********************************************************************************/
void lcd_draw_buffer(uint8_t *buffer, int x_start, int y_start, int width, int height) 
{
    uint16_t i;

    // TODO Fix the window to be more in the middle of the screen
    lcd_set_windows(x_start, y_start, x_start + width - 1, y_start + height - 1);

    gpio_put(LCD_PIN_DC, 1);
    gpio_put(LCD_PIN_CS, 0);

    for (i = 0; i< width * height; i++)
    {
        uint16_t colour = buffer[i];
        lcd_send_data(colour >> 8);
        lcd_send_data(colour & 0xFF);
    }
    gpio_put(LCD_PIN_CS, 1);
}


void lcd_draw_block(int x_start, int y_start, int width, int height, uint16_t colour) {
    // Send start/end commands for x and y, with DCX pin low?
    // TODO

    // Fill local buffer with colour
    // TODO

    // Send data with DCX pin high
    // TODO
}


static void decode_img(pw_img_t *pw_img, size_t out_len, uint8_t out_buf[out_len]) {

    uint8_t pixel_value, bpu, bpl;
    size_t row, col, stride = pw_img->height;

    pw_img->size = pw_img->width * pw_img->height * 2/8;

    // quit if the output buffer can't hold all the data
    if(out_len < pw_img->size * 2*SCREEN_SCALE*SCREEN_SCALE) {
        printf("Error: Decoded image (%lu bytes) is larger than output buffer (%lu bytes)\n",
                pw_img->size * 2*SCREEN_SCALE*SCREEN_SCALE, out_len);
        return;
    }

    // i = number of bytes into image
    for(size_t i = 0; i < pw_img->size; i+= 2) {
        bpu = pw_img->data[i+0];
        bpl = pw_img->data[i+1];

        // j = index of pixel in chunk
        for(size_t j = 0; j < 8; j++) {
            pixel_value  = ((bpu>>j) & 1) << 1;
            pixel_value |= ((bpl>>j) & 1);

            // transform coords
            size_t x_normal = (i/2)%pw_img->width;
            size_t y_normal = 8*(i/(2*pw_img->width)) + j;
            //col = pw_img->height - y_normal - 1;
            //row = x_normal;
            col = y_normal;
            row = pw_img->width - x_normal - 1;

            // now we have pixel coordinate, write to all pixels
            // that need the colour
            for(size_t py = 0; py < SCREEN_SCALE; py++) {
                for(size_t px = 0; px < SCREEN_SCALE; px++) {
                    size_t base_index = SCREEN_SCALE*((SCREEN_SCALE*row+py)*stride) + SCREEN_SCALE*col+px;

                    out_buf[2*base_index+0] = colour_map[pixel_value]>>8;
                    out_buf[2*base_index+1] = colour_map[pixel_value]&0xff;
                    if(2*base_index > out_len) {
                        printf("Error: Decode img output out of bounds\n");
                        return;
                    }
                }
            }
        }
    }

}

// TODO Adjust this code so it's only rendering 128 x 96
screen_area_t transform_pw_to_lcd(screen_area_t pw_area, lcd_t a) {
    screen_area_t lcd_area = {0};
    //lcd_area.x = (SCREEN_HEIGHT - pw_area.height - pw_area.y)*SCREEN_SCALE + a.offset_x;
    lcd_area.x = pw_area.y * SCREEN_SCALE + a.offset_x;
    lcd_area.y = (SCREEN_WIDTH + 1 - pw_area.x - pw_area.width)*SCREEN_SCALE + a.offset_y;
    lcd_area.width = pw_area.height * SCREEN_SCALE;
    lcd_area.height = pw_area.width * SCREEN_SCALE;
    return lcd_area;
}

/*
 * ============================================================================
 * Driver functions
 * ============================================================================
 */

void pw_screen_init() 
{

    /*
     * SPI 4-wire (with mosi, miso, and D/C line)
     * MSB first, D/CX should be high on LSB if its data and low on command.
     * Can "cheat" and just leave it high for the whole SPI comms if sending data.
     * RESX should stay high in normal operation
     * Run at XX MHz
     */
    // TODO: set up SPI

    // Reset and clear memory
    lcd_reset();

    // Initialize LCD Screen
    lcd_init();

    // Clear Screen
    lcd_clear_screen();
}


void pw_screen_draw_img(pw_img_t *img, screen_pos_t x, screen_pos_t y) 
{
    // TODO: checks image isn't too large
    decode_img(img, LCD_BUFFER_SIZE, lcd_buffer);

    // Put decoded, transformed image in `lcd_buffer`
    decode_img(img, LCD_BUFFER_SIZE, lcd_buffer);

    // Transform image area to lcd coordinates
    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = img->width, .height = img->height};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);

    lcd_draw_buffer(lcd_buffer, lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*lcd_draw_buffer(
            //((SCREEN_HEIGHT-img->height-y)*SCREEN_SCALE)+lcd.offset_x, (x*SCREEN_SCALE)+lcd.offset_y,
            //img->height*SCREEN_SCALE, img->width*SCREEN_SCALE,
            lcd_area.x, lcd_area.y,
            lcd_area.width, lcd_area.height,
            2*lcd_area.width*lcd_area.height,
            lcd_buffer);*/
}


void pw_screen_clear_area(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height) 
{

    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = width, .height = height};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);

    // Clear Area via White Colour
    lcd_draw_buffer(colour_map[0], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[SCREEN_WHITE]
    );
    */
}

void pw_screen_draw_horiz_line(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_colour_t c) 
{
    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = w, .height = 1};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);
    
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */
}


void pw_screen_draw_text_box(screen_pos_t x1, screen_pos_t y1, screen_pos_t width, screen_pos_t height, screen_colour_t c) 
{

    // assume y2 > y1 and x2 > x1
    screen_pos_t x2 = x1 + width - 1;
    screen_pos_t y2 = y1 + height - 1;

    screen_area_t lcd_area = {0}, pw_area = {0};

    // top bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = width, .height = 1};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */

    // bottom bar
    pw_area = (screen_area_t){.x = x1, .y = y2, .width = width, .height = 1};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */

    // left bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = 1, .height = height};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */

    // right bar
    pw_area = (screen_area_t){.x = x2, .y = y1, .width = 1, .height = height};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */
}


void pw_screen_clear() 
{
    screen_area_t pw_area = (screen_area_t){.x=0, .y=0, .width=SCREEN_WIDTH, .height=SCREEN_HEIGHT};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[0], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[SCREEN_WHITE]
    );
    */
}


void pw_screen_fill_area(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_pos_t h, screen_colour_t c) 
{
    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = w, .height = h};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]
    );
    */
}

/********************************************************************************
Function: Turn off Display and enter Sleep Mode
Parameters:
*********************************************************************************/
void pw_screen_sleep() {
    // Enable display standby
    lcd_send_command(CMD_DISPLAY_OFF);
    sleep_ms(10);
    lcd_send_command(CMD_SLEEP_MODE);
    sleep_ms(100);
}

/********************************************************************************
Function: Removes Sleep Mode and turns on Display
Parameters:
*********************************************************************************/
void pw_screen_wake() {
    // Wake it up and re-configure it
    // Same as a power-on reset
    //lcd_reset();
    lcd_send_command(CMD_SLEEP_OUT);
    sleep_ms(100);
    lcd_send_command(CMD_DISPLAY_ON);
}
