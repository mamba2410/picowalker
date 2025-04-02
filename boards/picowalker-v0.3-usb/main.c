#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include <tusb.h>

#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

//#include "drivers/eeprom/m95512_rp2xxx_spi.h"
#include "picowalker-defs.h"

#include "board_resources.h"

static bool spi_is_inited = false;
static bool i2c_is_inited = false;

void board_spi_init() {
    if(spi_is_inited) return;

    spi_init(COMMON_SPI_HW, COMMON_SPI_SPEED_HZ);
    spi_set_format(COMMON_SPI_HW, 8, 0, 0, SPI_MSB_FIRST);
    gpio_set_function(COMMON_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(COMMON_SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(COMMON_SPI_MISO_PIN, GPIO_FUNC_SPI);

    spi_is_inited = true;
}

void board_i2c_init() {
    if(i2c_is_inited) return;
    i2c_init(COMMON_I2C_HW, COMMON_I2C_SPEED_HZ);
    gpio_set_function(COMMON_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(COMMON_I2C_SCK_PIN, GPIO_FUNC_I2C);
    // hardware pullups
    i2c_is_inited = true;
}


int main() {
    bi_decl(bi_program_description("picowalker"));

    // Downclock the system to save a bit of power and allow the HSTX
    // to have a lower clock
    //set_sys_clock_khz(48000, true);

    /*
    // emergency blink routine
    gpio_init(27);
    gpio_set_dir(27, GPIO_OUT);

    while(1) {
        gpio_put(27, 0);
        sleep_ms(1000);
        gpio_put(27, 1);
        sleep_ms(1000);
    }
    */



    stdio_init_all();

    sleep_ms(1000);

    printf("[Info] ==== Hello, picowalker! ====\n");

    /* Program the EEPROM over debug uart
    // Send a full rom file using `cat`
    // Beware: no validation is done
    pw_eeprom_init();

    uint16_t address = 0;
    uint8_t page[128];
    while(1){
        if(uart_is_readable(uart0)) {
            page[address&0x7f] = uart_getc(uart0);
            address++;
            if((address&0x7f) == 0) {
                pw_eeprom_write(address-128, page, 128);
                printf("Written page to 0x%04x\n", address-128);
            }
        }
    }
    */

    /*
    // Dump eeprom over debug port
    // picocom -b 115200 /dev/ttyACM0 -g logfile.bin
    pw_eeprom_init();

    uint32_t address = 0;
    uint8_t page[128];
    while(address < 0x10000) {
        pw_eeprom_read(address, page, 128);
        uart_write_blocking(uart0, page, 128);
        address += 128;
    }
    while(1);
    */

    //audio_test_program();

    // Start picowalker-core
    walker_setup();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };



    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init();

    extern void (*current_loop)(void);

    while(1) {
        tud_task();
        current_loop();
        cdc_task();
    }

    // unreachable
    while(1);
}

void tud_mount_cb(void) {
    printf("[Info] tusb mounted\n");

    // Halt sleep timer so we don't mess up tusb
    power_sleep_enabled = false;

}

void tud_umount_cb(void) {
    printf("[Info] tusb unmounted\n");

    // Unmount and now we're safe to sleep
    power_sleep_enabled = true;
    set_user_idle_timer();
}

void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }
void tud_resume_cb(void) {}

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

// Invoked when cdc when line state changed e.g connected/disconnected
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

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
  (void) itf;
}

