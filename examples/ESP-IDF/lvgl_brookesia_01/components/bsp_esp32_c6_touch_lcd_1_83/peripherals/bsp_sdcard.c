#include "bsp_board.h"

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "sdmmc_cmd.h"
#include "dirent.h"

#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_check.h"
#include "esp_vfs_fat.h"

static const char *TAG = "bsp sdcard";
static uint32_t sdcard_total_size  = 0;

esp_err_t sd_card_init(void)
{
    esp_err_t ret = ESP_OK;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();


    /*spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_SDSPI_MOSI,
        .miso_io_num = GPIO_SDSPI_MISO,
        .sclk_io_num = GPIO_SDSPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }*/

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_SDSPI_CS;
    slot_config.host_id = EXAMPLE_LCD_SPI_NUM;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));

        }
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    sdcard_total_size = ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024);

    return ret;
}

uint32_t get_sdcard_total_size(void)
{
    return sdcard_total_size;
}

uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][MAX_FILE_NAME_SIZE], uint16_t maxFiles)    
{
    DIR *dir = opendir(directory);  
    if (dir == NULL) {
        ESP_LOGE(TAG, "Path: <%s> does not exist", directory); 
        return 0;  
    }

    uint16_t fileCount = 0;  
    struct dirent *entry;   

    while ((entry = readdir(dir)) != NULL && fileCount < maxFiles) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        const char *dot = strrchr(entry->d_name, '.');  
        if (dot != NULL && dot != entry->d_name) { 

            if (strcasecmp(dot, fileExtension) == 0) {  
                strlcpy(File_Name[fileCount], entry->d_name, MAX_FILE_NAME_SIZE);
                File_Name[fileCount][MAX_FILE_NAME_SIZE - 1] = '\0';  

                char filePath[MAX_PATH_SIZE];
                snprintf(filePath, MAX_PATH_SIZE, "%s/%s", directory, entry->d_name);

                printf("File found: %s\r\n", filePath); 
                fileCount++;  
            }
        }
        else{
           
            // printf("No extension found for file: %s\r\n", entry->d_name);
        }
    }

    closedir(dir);  

    if (fileCount > 0) {
        ESP_LOGI(TAG, "Retrieved %d files with extension '%s'", fileCount, fileExtension);  
    } else {
        ESP_LOGW(TAG, "No files with extension '%s' found in directory: %s", fileExtension, directory); 
    }

    return fileCount; 
}