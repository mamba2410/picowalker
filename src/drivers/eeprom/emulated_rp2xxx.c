#include "emulated_rp2xxx.h"

// Pico Library
#include "string.h"
#include <stdio.h>

// EEPROM emulation state
static bool eeprom_initialized = false;
static uint8_t sector_buffer[FLASH_SECTOR_SIZE];

// RAM-based EEPROM cache with lazy loading
static uint8_t eeprom_ram_cache[DRIVER_EEPROM_SIZE];
static bool eeprom_cache_loaded = false;  // Is cache currently loaded in RAM?
static bool eeprom_cache_dirty = false;   // Does cache differ from flash?

// Cache control: Enable/disable caching dynamically
// When disabled, reads/writes go directly to flash (slower but saves RAM access)
// When enabled, operations use RAM cache (fast, needed for IR transfers)
static bool eeprom_cache_enabled = false;


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
    
    printf("[EEPROM] Initializing RAM-cached EEPROM emulation...\n");
    printf("[EEPROM] Using flash offset: 0x%08X, size: %d bytes\n", 
           DRIVER_EEPROM_FLASH_OFFSET, DRIVER_EEPROM_SIZE);
    
    // Load entire EEPROM from flash into RAM cache
    memcpy(eeprom_ram_cache, (uint8_t*)(XIP_BASE + DRIVER_EEPROM_FLASH_OFFSET), DRIVER_EEPROM_SIZE);
    
    // Check if the EEPROM area looks valid (simple check for all 0xFF = erased)
    uint8_t first_bytes[16];
    memcpy(first_bytes, eeprom_ram_cache, 16);
    
    bool all_erased = true;
    for (int i = 0; i < 16; i++) 
    {
        if (first_bytes[i] != 0xFF) 
        {
            all_erased = false;
            break;
        }
    }
    
    eeprom_cache_dirty = false;  // Cache matches flash initially
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
    if (addr + len > DRIVER_EEPROM_SIZE) 
    {
        printf("[EEPROM] Read beyond EEPROM bounds: addr=0x%04X, len=%d\n", addr, len);
        return -1;
    }

    if (!eeprom_initialized) pw_eeprom_init();

    // If cache is enabled and loaded, read from RAM (fast)
    if (eeprom_cache_enabled && eeprom_cache_loaded) 
    {
        memcpy(buf, &eeprom_ram_cache[addr], len);
        return 0;
    }

    // Otherwise, read directly from flash (slower but no RAM needed)
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
    if (addr + len > DRIVER_EEPROM_SIZE) 
    {
        printf("[EEPROM] Write beyond EEPROM bounds: addr=0x%04X, len=%d\n", addr, len);
        return -1;
    }

    if (!eeprom_initialized) pw_eeprom_init();

    // If cache is enabled and loaded, write to RAM (fast, flush later)
    if (eeprom_cache_enabled && eeprom_cache_loaded) 
    {
        memcpy(&eeprom_ram_cache[addr], buf, len);
        eeprom_cache_dirty = true;
        // printf("[EEPROM] Written %d bytes to RAM cache at 0x%04X\n", len, addr);
        return 0;
    }

    // Otherwise, write directly to flash (slower, immediate persistence)
    // This requires reading entire sectors, modifying, and writing back
    printf("[EEPROM] Direct flash write: %d bytes at 0x%04X (slow!)\n", len, addr);

    uint32_t start_addr = DRIVER_EEPROM_FLASH_OFFSET + addr;
    uint32_t end_addr = start_addr + len - 1;
    uint32_t start_sector = start_addr / FLASH_SECTOR_SIZE;
    uint32_t end_sector = end_addr / FLASH_SECTOR_SIZE;

    for (uint32_t sector = start_sector; sector <= end_sector; sector++) 
    {
        uint32_t sector_offset = sector * FLASH_SECTOR_SIZE;

        // Read entire sector
        memcpy(sector_buffer, (uint8_t*)(XIP_BASE + sector_offset), FLASH_SECTOR_SIZE);

        // Calculate modification range within this sector
        uint32_t sector_start = (sector_offset > start_addr) ? 0 : (start_addr - sector_offset);
        uint32_t sector_end = ((sector_offset + FLASH_SECTOR_SIZE) < (end_addr + 1)) ?
                              FLASH_SECTOR_SIZE : ((end_addr + 1) - sector_offset);
        uint32_t buf_offset = (sector_offset >= start_addr) ? (sector_offset - start_addr) : 0;

        // Modify sector buffer
        memcpy(&sector_buffer[sector_start], &buf[buf_offset], sector_end - sector_start);

        // Erase and write back
        uint32_t interrupts = save_and_disable_interrupts();
        flash_range_erase(sector_offset, FLASH_SECTOR_SIZE);
        flash_range_program(sector_offset, sector_buffer, FLASH_SECTOR_SIZE);
        restore_interrupts(interrupts);
    }

    printf("[EEPROM] Direct write complete\n");
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
    if (!temp_buf) 
    {
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

/********************************************************************************
 * @brief           Check if EEPROM cache is dirty (differs from flash)
 * @return bool     true if cache has unsaved changes
********************************************************************************/
bool pw_eeprom_is_cache_dirty()
{
    return eeprom_cache_dirty;
}

/********************************************************************************
 * @brief           Get direct pointer to RAM cache (read-only access)
 * @return uint8_t* Pointer to RAM cache
********************************************************************************/
const uint8_t* pw_eeprom_get_cache_ptr()
{
    if (!eeprom_initialized) pw_eeprom_init();
    return eeprom_ram_cache;
}

/********************************************************************************
 * @brief           Write entire EEPROM cache to flash
 * @return int      0 on success, -1 on error
********************************************************************************/
int pw_eeprom_flush_to_flash()
{
    if (!eeprom_initialized) 
    {
        printf("[EEPROM] Cannot flush: EEPROM not initialized\n");
        return -1;
    }
    
    if (!eeprom_cache_dirty) 
    {
        printf("[EEPROM] Cache is clean, no need to flush\n");
        return 0;
    }
    
    printf("[EEPROM] Flushing entire RAM cache to flash...\n");
    
    // Calculate which flash sectors we need to update
    uint32_t start_sector = DRIVER_EEPROM_FLASH_OFFSET / FLASH_SECTOR_SIZE;
    uint32_t end_sector = (DRIVER_EEPROM_FLASH_OFFSET + DRIVER_EEPROM_SIZE - 1) / FLASH_SECTOR_SIZE;
    
    for (uint32_t sector = start_sector; sector <= end_sector; sector++) 
    {
        uint32_t sector_offset = sector * FLASH_SECTOR_SIZE;
        uint32_t eeprom_offset_in_sector = DRIVER_EEPROM_FLASH_OFFSET - sector_offset;
        
        // Read the entire sector from flash
        memcpy(sector_buffer, (uint8_t*)(XIP_BASE + sector_offset), FLASH_SECTOR_SIZE);
        
        // Calculate how much of this sector contains EEPROM data
        uint32_t eeprom_start_in_sector = (sector_offset <= DRIVER_EEPROM_FLASH_OFFSET) ? 
                                         eeprom_offset_in_sector : 0;
        uint32_t eeprom_end_in_sector = ((sector_offset + FLASH_SECTOR_SIZE) >= 
                                        (DRIVER_EEPROM_FLASH_OFFSET + DRIVER_EEPROM_SIZE)) ?
                                       (DRIVER_EEPROM_FLASH_OFFSET + DRIVER_EEPROM_SIZE - sector_offset) :
                                       FLASH_SECTOR_SIZE;
        
        // Calculate corresponding offset in RAM cache
        uint32_t cache_offset = (sector_offset > DRIVER_EEPROM_FLASH_OFFSET) ?
                               (sector_offset - DRIVER_EEPROM_FLASH_OFFSET) : 0;
        
        // Copy EEPROM data from RAM cache to sector buffer
        if (eeprom_end_in_sector > eeprom_start_in_sector) 
        {
            memcpy(&sector_buffer[eeprom_start_in_sector],
                   &eeprom_ram_cache[cache_offset],
                   eeprom_end_in_sector - eeprom_start_in_sector);
        }
        
        // Erase and write back the sector
        uint32_t interrupts = save_and_disable_interrupts();
        flash_range_erase(sector_offset, FLASH_SECTOR_SIZE);
        flash_range_program(sector_offset, sector_buffer, FLASH_SECTOR_SIZE);
        restore_interrupts(interrupts);
        
        printf("[EEPROM] Flushed sector at 0x%08X\n", sector_offset);
    }
    
    eeprom_cache_dirty = false;
    printf("[EEPROM] Flash write complete, cache is now clean\n");
    
    return 0;
}

/********************************************************************************
 * @brief           Reload EEPROM cache from flash (discarding RAM changes)
 * @return int      0 on success, -1 on error
********************************************************************************/
int pw_eeprom_reload_from_flash()
{
    if (!eeprom_initialized) 
    {
        printf("[EEPROM] Cannot reload: EEPROM not initialized\n");
        return -1;
    }
    
    printf("[EEPROM] Reloading RAM cache from flash (discarding changes)...\n");
    
    // Reload entire EEPROM from flash into RAM cache
    memcpy(eeprom_ram_cache, (uint8_t*)(XIP_BASE + DRIVER_EEPROM_FLASH_OFFSET), DRIVER_EEPROM_SIZE);
    
    eeprom_cache_dirty = false;
    printf("[EEPROM] Cache reloaded from flash\n");
    
    return 0;
}

/********************************************************************************
 * @brief           Read directly from flash (bypassing RAM cache)
 * @param addr      Address of EEPROM
 * @param buf       Buffer
 * @param len       Size of Buffer
 * @return int      0 on success, -1 on error
********************************************************************************/
int pw_eeprom_read_flash_direct(eeprom_addr_t addr, uint8_t *buf, size_t len)
{
    if (addr + len > DRIVER_EEPROM_SIZE) 
    {
        printf("[EEPROM] Direct flash read beyond bounds: addr=0x%04X, len=%d\n", addr, len);
        return -1;
    }
    
    // Read directly from flash
    memcpy(buf, (uint8_t*)(XIP_BASE + DRIVER_EEPROM_FLASH_OFFSET + addr), len);
    return 0;
}

/********************************************************************************
 * @brief           Get EEPROM size
 * @return size_t   EEPROM size in bytes
********************************************************************************/
size_t pw_eeprom_get_size()
{
    return DRIVER_EEPROM_SIZE;
}

/********************************************************************************
 * @brief           Enable RAM caching (for fast IR transfers)
 *                  Loads EEPROM from flash to RAM if not already loaded
 * @return int      0 on success, -1 on error
********************************************************************************/
int pw_eeprom_enable_cache()
{
    if (!eeprom_initialized) pw_eeprom_init();

    if (!eeprom_cache_loaded) 
    {
        printf("[EEPROM] Loading cache from flash...\n");
        memcpy(eeprom_ram_cache, (uint8_t*)(XIP_BASE + DRIVER_EEPROM_FLASH_OFFSET), DRIVER_EEPROM_SIZE);
        eeprom_cache_loaded = true;
        eeprom_cache_dirty = false;
        printf("[EEPROM] Cache loaded (%d bytes)\n", DRIVER_EEPROM_SIZE);
    }

    eeprom_cache_enabled = true;
    printf("[EEPROM] Cache mode ENABLED (fast operations)\n");
    return 0;
}

/********************************************************************************
 * @brief           Disable RAM caching and flush to flash if dirty
 *                  Returns to direct flash access mode
 * @return int      0 on success, -1 on error
********************************************************************************/
int pw_eeprom_disable_cache()
{
    if (eeprom_cache_enabled && eeprom_cache_dirty) 
    {
        printf("[EEPROM] Flushing dirty cache before disabling...\n");
        pw_eeprom_flush_to_flash();
    }

    eeprom_cache_enabled = false;
    printf("[EEPROM] Cache mode DISABLED (direct flash access)\n");
    return 0;
}