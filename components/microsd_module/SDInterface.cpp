#include "SDCardInterface.hpp"

#define SD_MOSI 23
#define SD_MISO 19
#define SD_CLK 18
#define SD_CS 15

static constexpr const char *TAG = "SDCARD";

void SDCardInterface::append_cbor_to_sd(uint8_t *buffer, size_t len)
{
    FILE *f = fopen("/sdcard/logs.bin", "ab");

    if (!f)
    {
        ESP_LOGI(TAG, "File open fail.");
        return;
    }

    fwrite(&len, 1, sizeof(len), f);
    fwrite(buffer, 1, len, f);
    fclose(f);
}

void SDCardInterface::read_cbor_from_sd(uint8_t *buffer, size_t bufferSize)
{

    FILE *f = fopen("/sdcard/logs.bin", "rb");

    if (!f)
    {
        ESP_LOGI("TAG", "File open fail.");
        return;
    }

    size_t bufferCursor = 0;

    for (int i = 0; i < 10; i++)
    {
        uint32_t len = 0;
        size_t readLen = fread(&len, 1, sizeof(len), f);
        if (readLen != sizeof(len))
        {
            return;
        }
        if (bufferCursor + len <= bufferSize)
        {
            fread(buffer + bufferCursor, 1, len, f);
            bufferCursor += len;
        }
        else
        {
            return;
        }
    }
}

void SDCardInterface::init_sdcard()
{
    vTaskDelay(pdMS_TO_TICKS(5000));

    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";
    esp_err_t ret;

    // ---- 1. Init SPI bus ----
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = 400; // SPI hızını 10KHz yap
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI,
        .miso_io_num = SD_MISO,
        .sclk_io_num = SD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        printf("SPI bus init failed!\n");
        return;
    }

    // ---- 2. Configure CS pin for SD card ----
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = (spi_host_device_t)host.slot;
    slot_config.gpio_cs = (gpio_num_t)SD_CS;

    // ---- 3. Mount FAT filesystem ----
    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config,
                                  &mount_cfg, &card);
    if (ret != ESP_OK)
    {
        printf("SD card mount failed!\n");
        return;
    }

    printf("SD card mounted.\n");

    sdmmc_card_print_info(stdout, card);
}
