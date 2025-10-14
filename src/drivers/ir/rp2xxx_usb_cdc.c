#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "pico/time.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "rp2xxx_usb_cdc.h"
#include "../eeprom/emulated_rp2xxx.h"  // For EEPROM cache control

/*
 * USB CDC IR Communication Driver
 *
 * This driver provides IR communication over USB CDC (virtual serial port)
 * for compatibility with melonDS-IR emulator's Serial Port mode.
 *
 * Protocol details (from melonDS-IR):
 * - Baud rate: 115200 (configured on host side)
 * - Data format: 8N1 (8 data bits, no parity, 1 stop bit)
 * - Encryption: All bytes XOR'd with 0xAA (handled by higher layers)
 * - Timing: Must respond within 3.5ms to avoid timeout
 * - Max packet size: 184 bytes (0xB8)
 */

#define CDC_ITF 1               // CDC interface 1 for IR (CDC 0 is used for stdio/debug)
#define RX_TIMEOUT_US 50000     // 50ms timeout for initial data
#define BYTE_TIMEOUT_US 3742    // ~3.7ms timeout between bytes (matches melonDS-IR's 3500us window)

// Internal RX buffer for CDC data
#define RX_BUFFER_SIZE 256
static uint8_t g_rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t g_rx_write_pos = 0;
static volatile uint16_t g_rx_read_pos = 0;

// State tracking
static bool g_usb_cdc_initialized = false;


/********************************************************************************
 * @brief           Check if RX buffer is empty
 * @return          true if buffer is empty, false otherwise
 ********************************************************************************/
static inline bool rx_buffer_is_empty(void) 
{
    return g_rx_read_pos == g_rx_write_pos;
}


/********************************************************************************
 * @brief           Get number of bytes available in RX buffer
 * @return          Number of bytes available to read
 ********************************************************************************/
static inline uint16_t rx_buffer_available(void) 
{
    return (g_rx_write_pos - g_rx_read_pos) & (RX_BUFFER_SIZE - 1);
}


/********************************************************************************
 * @brief           Read one byte from RX buffer
 * @return          Byte value (0-255) or -1 if buffer is empty
 ********************************************************************************/
static int16_t rx_buffer_get(void) 
{
    if (rx_buffer_is_empty()) 
    {
        return -1;
    }
    uint8_t byte = g_rx_buffer[g_rx_read_pos];
    g_rx_read_pos = (g_rx_read_pos + 1) & (RX_BUFFER_SIZE - 1);
    return byte;
}

/********************************************************************************
 * @brief           Add one byte to RX buffer
 * @param byte      Byte to add to buffer
 * @return          true if successful, false if buffer is full
 ********************************************************************************/
static bool rx_buffer_put(uint8_t byte) 
{
    uint16_t next_write = (g_rx_write_pos + 1) & (RX_BUFFER_SIZE - 1);
    if (next_write == g_rx_read_pos) 
    {
        return false; // Buffer full
    }
    g_rx_buffer[g_rx_write_pos] = byte;
    g_rx_write_pos = next_write;
    return true;
}

/********************************************************************************
 * @brief           Clear the RX buffer (ring buffer and TinyUSB CDC buffer)
 *                  Discards stale data from failed connection attempts
 *                  Also flushes EEPROM cache if dirty
 ********************************************************************************/
void pw_ir_clear_rx(void)
{
    // Clear our internal ring buffer
    g_rx_read_pos = 0;
    g_rx_write_pos = 0;

    // Also flush TinyUSB's CDC RX buffer to discard any stale data
    if (g_usb_cdc_initialized) 
    {
        uint32_t flushed = 0;
        while (tud_cdc_n_available(CDC_ITF) > 0) 
        {
            tud_cdc_n_read_char(CDC_ITF);
            flushed++;
            if (flushed > 256) break; // Safety limit
        }
        if (flushed > 0) 
        {
            printf("[IR-CDC] Flushed %lu stale bytes from USB CDC buffer\n", flushed);
        }
    }

    // If EEPROM cache is dirty, flush it now
    // This ensures data is saved after IR transfer completes or fails
    if (pw_eeprom_is_cache_dirty()) 
    {
        printf("[IR-CDC] Flushing dirty EEPROM cache after IR operation\n");
        pw_eeprom_flush_to_flash();
    }
}

/********************************************************************************
 * @brief           Service the USB CDC interface
 *                  Must be called regularly to transfer data from USB CDC
 *                  into internal ring buffer
 ********************************************************************************/
static void service_usb_cdc(void)
{
    if (!g_usb_cdc_initialized) return;

    // Check if CDC is connected and has data
    if (tud_cdc_n_connected(CDC_ITF) && tud_cdc_n_available(CDC_ITF))
    {
        while (tud_cdc_n_available(CDC_ITF))
        {
            uint8_t byte = tud_cdc_n_read_char(CDC_ITF);
            if (!rx_buffer_put(byte))
            {
                // Buffer full - data will be lost
                // In practice this shouldn't happen with proper timing
                break;
            }
        }
    }
}

/********************************************************************************
 * @brief           Initialize USB CDC IR driver
 *                  Note: USB stack (tusb_init) must already be initialized
 *                  Clears RX buffer and enables EEPROM cache for fast transfers
 ********************************************************************************/
void pw_ir_init(void)
{
    // Always clear RX buffer, even if already initialized
    pw_ir_clear_rx();

    // Enable EEPROM cache for fast IR transfers
    // This loads the EEPROM into RAM so read/write operations are fast
    pw_eeprom_enable_cache();

    if (g_usb_cdc_initialized) 
    {
        printf("[IR-CDC] Already initialized, buffers cleared for new connection\n");
        return;
    }

    g_usb_cdc_initialized = true;

    printf("[IR-CDC] USB CDC IR driver initialized (using CDC instance %d)\n", CDC_ITF);
    printf("[IR-CDC] CDC 0 connected: %d\n", tud_cdc_n_connected(0));
    printf("[IR-CDC] CDC 1 connected: %d\n", tud_cdc_n_connected(1));

    // Wait for USB CDC to be ready (optional, but helpful)
    // This ensures the host has enumerated the device
    absolute_time_t start = get_absolute_time();
    while (!tud_cdc_n_connected(CDC_ITF)) 
    {
        tud_task(); // Service USB stack
        if (absolute_time_diff_us(start, get_absolute_time()) > 1000000) 
        {
            // 1 second timeout - continue anyway
            printf("[IR-CDC] Timeout waiting for CDC %d connection\n", CDC_ITF);
            printf("[IR-CDC] CDC 0 connected: %d\n", tud_cdc_n_connected(0));
            printf("[IR-CDC] CDC 1 connected: %d\n", tud_cdc_n_connected(1));
            break;
        }
        sleep_ms(10);
    }

    // if (tud_cdc_n_connected(CDC_ITF)) 
    // {
    //     printf("[IR-CDC] CDC %d connected and ready!\n", CDC_ITF);
    // }
}

/********************************************************************************
 * @brief           Deinitialize USB CDC IR driver
 *                  Flushes EEPROM cache if dirty and clears buffers
 ********************************************************************************/
void pw_ir_deinit(void)
{
    if (!g_usb_cdc_initialized) return;

    pw_ir_clear_rx();

    g_usb_cdc_initialized = false;
}

/********************************************************************************
 * @brief           Read data from USB CDC
 *                  Waits up to 50ms for initial data, then collects bytes
 *                  until 3.7ms timeout between bytes
 * @param buf       Buffer to store received data
 * @param max_len   Maximum number of bytes to read
 * @return          Number of bytes read (0 if timeout with no data)
 ********************************************************************************/
int pw_ir_read(uint8_t *buf, size_t max_len) 
{
    if (!g_usb_cdc_initialized || !buf) 
    {
        printf("[IR-CDC] Read called but not initialized or null buffer\n");
        return 0;
    }

    size_t bytes_read = 0;
    absolute_time_t start, last_byte_time, now;
    int64_t diff;

    // Wait for initial data to arrive (up to 50ms)
    start = get_absolute_time();
    do
    {
        tud_task(); // Service USB stack
        service_usb_cdc(); // Transfer USB data to our buffer
        now = get_absolute_time();
        diff = absolute_time_diff_us(start, now);
    } while (rx_buffer_is_empty() && diff < RX_TIMEOUT_US);

    // If no data arrived, return 0
    if (rx_buffer_is_empty()) 
    {
        // Only print occasionally to avoid spam
        static uint32_t timeout_count = 0;
        if (++timeout_count % 100 == 0) 
        {
            printf("[IR-CDC] Read timeout (no data after 50ms) - count: %lu\n", timeout_count);
        }
        return 0;
    }

    // Data has started arriving - collect bytes until timeout
    last_byte_time = get_absolute_time();
    diff = 0;

    do
    {
        tud_task(); // Service USB stack
        service_usb_cdc(); // Transfer USB data to our buffer

        // TODO Test the change in picowalker-core and see if changing the packet->cmd
        // to packet->extra works. We may not need the code below...

        // Try to read a byte
        int16_t byte = rx_buffer_get();
        if (byte >= 0)
        {
            buf[bytes_read++] = (uint8_t)byte;
            last_byte_time = get_absolute_time();

            // Packet boundary detection: If we've read exactly 8 bytes AND
            // there are exactly 8 more bytes buffered, this is likely
            // CMD_WALK_END_REQ (8 bytes) + CMD_DISCONNECT (8 bytes) sent together.
            // Stop after the first packet to avoid the "length 16" error.
            if (bytes_read == 8 && rx_buffer_available() == 8)
            {
                printf("[IR-CDC] Packet boundary detected: read 8 bytes, 8 more buffered (likely separate packets)\n");
                break;
            }

            // Stop if buffer is full
            if (bytes_read >= max_len) break;

        }

        now = get_absolute_time();
        diff = absolute_time_diff_us(last_byte_time, now);
    } while (diff < BYTE_TIMEOUT_US);

    // Debug Commands...
    // if (bytes_read >= 1) 
    // {
    //     uint8_t cmd = buf[0] ^ 0xAA;  // Decrypt first byte
    //     printf("[IR-CDC] CMD=0x%02X\n", cmd);
    // }

    return bytes_read;
}


/********************************************************************************
 * @brief           Write data to USB CDC
 *                  Sends data immediately, blocks until sent or timeout
 *                  Flushes EEPROM cache after CMD_WALK_END_ACK or CMD_WALK_START
 * @param buf       Buffer containing data to send
 * @param len       Number of bytes to send
 * @return          Number of bytes written
 ********************************************************************************/
int pw_ir_write(uint8_t *buf, size_t len) 
{
    if (!g_usb_cdc_initialized || !buf || len == 0) return 0;
    // // Wait for CDC to be connected
    absolute_time_t start = get_absolute_time();


    if (!tud_cdc_n_connected(CDC_ITF))
    {
        printf("[IR-CDC] WARNING: Writing %d bytes while CDC %d not connected (DTR not set)\n", len, CDC_ITF);
        // Continue anyway - USB CDC doesn't require DTR to send data
    }
    
    // Send data in chunks if needed (CDC buffer might be smaller than len)
    size_t total_written = 0;
    while (total_written < len) 
    {
        tud_task(); // Service USB stack

        // Calculate how much we can send in this iteration
        size_t remaining = len - total_written;
        size_t available = tud_cdc_n_write_available(CDC_ITF);
        size_t to_write = (remaining < available) ? remaining : available;

        if (to_write > 0) 
        {
            uint32_t written = tud_cdc_n_write(CDC_ITF, buf + total_written, to_write);
            total_written += written;
        }

        // Flush to ensure data goes out immediately
        tud_cdc_n_write_flush(CDC_ITF);

        // Timeout check (shouldn't take more than 10ms to send at 115200 baud)
        if (absolute_time_diff_us(start, get_absolute_time()) > 10000) break;
    }

    // Final flush
    tud_cdc_n_write_flush(CDC_ITF);

    // At 115200 baud: ~87 microseconds per byte
    // Add extra margin for USB overhead and host processing
    uint32_t transmission_time_us = (total_written * 87) + 5000; // +5ms safety margin
    busy_wait_us(transmission_time_us);

    // Detect (CMD_WALK_END_ACK) 0x50, (CMD_WALK_START) 0x5a
    // This is the final acknowledgment sent by the walker to the game
    // Flush EEPROM cache immediately to ensure all data is saved
    // This is temporary until pw_ir_deinit() is developed in picowalker-core
    // Saving is happening too early it seems
    if (total_written >= 1) 
    {
        uint8_t cmd = buf[0] ^ 0xAA;  // Decrypt first byte
        // printf("[IR-CDC] CMD=0x%02X\n", cmd);
        if (cmd == 0x50) printf("[IR-CDC] CMD=0x%02X CMD_WALK_END_ACK\n", cmd);
        if (cmd == 0x5A) printf("[IR-CDC] CMD=0x%02X CMD_WALK_START\n", cmd);
        if (cmd == 0x50 || cmd == 0x5A) // CMD_WALK_END_ACK
        {  
            if (pw_eeprom_is_cache_dirty()) 
            {
                pw_eeprom_flush_to_flash();
            }
        }
    }

    return total_written;
}

/********************************************************************************
 * @brief           Put IR driver to sleep (not implemented for USB CDC)
 ********************************************************************************/
void pw_ir_sleep()
{

}

/********************************************************************************
 * @brief           Wake IR driver from sleep (not implemented for USB CDC)
 ********************************************************************************/
void pw_ir_wake()
{

}