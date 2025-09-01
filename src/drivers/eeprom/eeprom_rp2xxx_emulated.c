#include "eeprom_rp2xxx_emulated.h"

// Pico Library
#include "string.h"
#include <stdio.h>

// EEPROM emulation state
static bool eeprom_initialized = false;
static uint8_t sector_buffer[FLASH_SECTOR_SIZE];


/*
 * ============================================================================
 * Picowalker Driver Functions
 * ============================================================================
 */


/********************************************************************************
 * @brief           Initialize EEPROM emulation (check if flash area is formatted)
 * @param N/A
********************************************************************************/
void pw_eeprom_init()
{
    if (eeprom_initialized) return;
    
    printf("[EEPROM] Initializing flash-based EEPROM emulation...\n");
    printf("[EEPROM] Using flash offset: 0x%08X, size: %d bytes\n", 
           DRIVER_EEPROM_FLASH_OFFSET, DRIVER_EEPROM_SIZE);
    
    // Check if the EEPROM area looks valid (simple check for all 0xFF = erased)
    uint8_t first_bytes[16];
    memcpy(first_bytes, (uint8_t*)(XIP_BASE + DRIVER_EEPROM_FLASH_OFFSET), 16);
    
    bool all_erased = true;
    for (int i = 0; i < 16; i++) {
        if (first_bytes[i] != 0xFF) {
            all_erased = false;
            break;
        }
    }
    
    if (all_erased) {
        printf("[EEPROM] Flash area appears erased, will initialize on first write\n");
    } else {
        printf("[EEPROM] Flash area contains data, using existing content\n");
    }
    
    eeprom_initialized = true;
}

/********************************************************************************
 * @brief           Reads from EEPROM
 * @param addr      Address of EEPROM
 * @param buf       Buffer
 * @param len       Size of Buffer
 * @return int
********************************************************************************/
int pw_eeprom_read(eeprom_addr_t addr, uint8_t *buf, size_t len) 
{
    // Read from flash at EEPROM_FLASH_OFFSET + addr
    memcpy(buf, (uint8_t*)(XIP_BASE + DRIVER_EEPROM_FLASH_OFFSET + addr), len);
    return 0;
}

/********************************************************************************
 * @brief           Writes to EEPROM (with proper sector management)
 * @param addr      Address of EEPROM
 * @param buf       Buffer
 * @param len       Size of Buffer
 * @return int
********************************************************************************/
int pw_eeprom_write(eeprom_addr_t addr, uint8_t *buf, size_t len) 
{
    if (addr + len > DRIVER_EEPROM_SIZE) {
        printf("[EEPROM] Write beyond EEPROM bounds: addr=0x%04X, len=%d\n", addr, len);
        return -1;
    }
    
    if (!eeprom_initialized) {
        pw_eeprom_init();
    }
    
    // Calculate which flash sector(s) we need to modify
    uint32_t start_sector = (DRIVER_EEPROM_FLASH_OFFSET + addr) / FLASH_SECTOR_SIZE;
    uint32_t end_sector = (DRIVER_EEPROM_FLASH_OFFSET + addr + len - 1) / FLASH_SECTOR_SIZE;
    
    for (uint32_t sector = start_sector; sector <= end_sector; sector++) {
        uint32_t sector_offset = sector * FLASH_SECTOR_SIZE;
        uint32_t sector_addr = sector_offset - DRIVER_EEPROM_FLASH_OFFSET;
        
        // Read entire sector into buffer
        memcpy(sector_buffer, (uint8_t*)(XIP_BASE + sector_offset), FLASH_SECTOR_SIZE);
        
        // Modify the relevant portion
        uint32_t write_start_in_sector = (addr > sector_addr) ? (addr - sector_addr) : 0;
        uint32_t write_end_in_sector = ((addr + len - 1) > (sector_addr + FLASH_SECTOR_SIZE - 1)) ? 
                                      FLASH_SECTOR_SIZE : (addr + len - sector_addr);
        uint32_t buf_offset = (sector_addr > addr) ? (sector_addr - addr) : 0;
        
        memcpy(&sector_buffer[write_start_in_sector], 
               &buf[buf_offset], 
               write_end_in_sector - write_start_in_sector);
        
        // Erase and write back the sector
        uint32_t interrupts = save_and_disable_interrupts();
        flash_range_erase(sector_offset, FLASH_SECTOR_SIZE);
        flash_range_program(sector_offset, sector_buffer, FLASH_SECTOR_SIZE);
        restore_interrupts(interrupts);
        
        printf("[EEPROM] Updated sector at 0x%08X\n", sector_offset);
    }
    
    return 0;
}

/********************************************************************************
 * @brief           Set an area of EEPROM to a specific value
 * @param addr      Address of EEPROM
 * @param v         Value to Set
 * @param len       Size of Value
********************************************************************************/
void pw_eeprom_set_area(eeprom_addr_t addr, uint8_t v, size_t len)
{
    if (len == 0) return;
    
    // Create a buffer filled with the value
    uint8_t *temp_buf = malloc(len);
    if (!temp_buf) {
        printf("[EEPROM] Failed to allocate temp buffer for set_area\n");
        return;
    }
    
    memset(temp_buf, v, len);
    pw_eeprom_write(addr, temp_buf, len);
    free(temp_buf);
    
    printf("[EEPROM] Set %d bytes at 0x%04X to 0x%02X\n", len, addr, v);
}

/********************************************************************************
 * @brief           EEPROM Sleep
 * @param N/A
********************************************************************************/
void pw_eeprom_sleep()
{

}

/********************************************************************************
 * @brief           EEPROM Wake
 * @param N/A
********************************************************************************/
void pw_eeprom_wake()
{

}