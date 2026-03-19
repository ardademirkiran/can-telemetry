
#include "rtc_driver.h"


static const char *TAG = "RTC";

#define COM_READ_SECONDS        0x81
#define COM_READ_MINUTES        0x83
#define COM_READ_HOUR           0x85
#define COM_READ_DAY_OF_MONTH   0x87
#define COM_READ_MONTH          0x89
#define COM_READ_YEAR           0x8D
#define COM_READ_WP             0x8F 
#define COM_READ_BURST          0xBF

#define COM_WRITE_WP             0x8E   
#define COM_WRITE_SECONDS        0x80
#define COM_WRITE_MINUTES        0x82
#define COM_WRITE_HOUR           0x84
#define COM_WRITE_DAY_OF_MONTH   0x86
#define COM_WRITE_MONTH          0x88
#define COM_WRITE_YEAR           0x8C
#define COM_WRITE_BURST          0xBE


#define PIN_CLK GPIO_NUM_25
#define PIN_DAT GPIO_NUM_26
#define PIN_RST GPIO_NUM_27


static inline void data_line_input() { gpio_set_direction(PIN_DAT, GPIO_MODE_INPUT); }
static inline void data_line_output() { gpio_set_direction(PIN_DAT, GPIO_MODE_OUTPUT); }

static inline void rst_line_high() { gpio_set_level(PIN_RST, 1); }
static inline void rst_line_low() { gpio_set_level(PIN_RST, 0); }

static inline void clk_line_high() { gpio_set_level(PIN_CLK, 1); }
static inline void clk_line_low() { gpio_set_level(PIN_CLK, 0); }

static inline void delay() { esp_rom_delay_us(2); }

void rtc_initialize_gpio(void) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    io_conf.pin_bit_mask = (1ULL << PIN_RST) |
                       (1ULL << PIN_CLK) |
                       (1ULL << PIN_DAT);

    gpio_config(&io_conf);

    gpio_set_level(PIN_DAT, 0);
    gpio_set_level(PIN_RST, 0);
    gpio_set_level(PIN_CLK, 0);
}

static void rtc_send_byte(uint8_t txBuf) {
    data_line_output();
    for (int i = 0; i < 8; i++) {
        clk_line_low();
        gpio_set_level(PIN_DAT, (txBuf >> i) & 0x01);
        delay();
        clk_line_high();
        delay();
    }
}


static uint8_t rtc_read_byte(){
    uint8_t value = 0;
    data_line_input();

    for (int i = 0; i < 8; i++) {
        clk_line_high();
        delay();
        clk_line_low();
        value |= (gpio_get_level(PIN_DAT) << i);
        delay();
    }
    return value;
}


static uint8_t bcd_to_dec(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t d)
{
    return ((d / 10) << 4) | (d % 10);
}


void sync_esp_time_from_dto(time_dto time_dto)
{
    struct tm tm_time = {0};

    tm_time.tm_sec  = time_dto.sec;
    tm_time.tm_min  = time_dto.min;
    tm_time.tm_hour = time_dto.hour;
    tm_time.tm_mday = time_dto.date;
    tm_time.tm_mon  = time_dto.month - 1;
    tm_time.tm_year = time_dto.year + 100;

    time_t t = mktime(&tm_time);

    struct timeval now = {
        .tv_sec = t,
        .tv_usec = 0
    };

    settimeofday(&now, NULL);
}


time_dto rtc_burst_read() {
    rst_line_high();
    delay();

    rtc_send_byte(COM_READ_BURST);
    uint8_t seconds, minutes, hours, date, month, dow, year, dummy_read; 
    
    seconds = rtc_read_byte();
    minutes = rtc_read_byte();
    hours = rtc_read_byte();
    date = rtc_read_byte();
    month = rtc_read_byte();
    dow = rtc_read_byte();
    year = rtc_read_byte();
    dummy_read = rtc_read_byte();

    rst_line_low();

    time_dto current_time_object;

    seconds &= 0x7F;
    current_time_object.sec = bcd_to_dec(seconds);
    minutes &= 0x7F;
    current_time_object.min = bcd_to_dec(minutes);
    hours &= 0x3F;
    current_time_object.hour = bcd_to_dec(hours);
    date &= 0x3F;
    current_time_object.date = bcd_to_dec(date);
    month &= 0x1F;
    current_time_object.month = bcd_to_dec(month);
    current_time_object.year = bcd_to_dec(year);

    ESP_LOGI(TAG,
         "%02d:%02d:%02d  %02d/%02d/%02d",
         current_time_object.hour,
         current_time_object.min,
         current_time_object.sec,
         current_time_object.date,
         current_time_object.month,
         current_time_object.year);


    return current_time_object;
}

void rtc_burst_write(time_dto time_object)
{
    rst_line_high();
    delay();


    rtc_send_byte(COM_WRITE_BURST);
    rtc_send_byte(dec_to_bcd(time_object.sec & 0x7F));

    rtc_send_byte(dec_to_bcd(time_object.min & 0x7F));
    rtc_send_byte(dec_to_bcd(time_object.hour& 0x3F));
    rtc_send_byte(dec_to_bcd(time_object.date & 0x3F));
    rtc_send_byte(dec_to_bcd(time_object.month & 0x1F));
    rtc_send_byte(dec_to_bcd(time_object.dow & 0x07));
    rtc_send_byte(dec_to_bcd(time_object.year));
    rtc_send_byte(0x00);

    rst_line_low();
}

void rtc_sync_device_time() {
    rtc_burst_read();
    delay();
    time_dto current_time_object = rtc_burst_read();
    sync_esp_time_from_dto(current_time_object);
}

void rtc_sync_clock_time(time_dto time_dto) {
        rtc_burst_write(time_dto);
}
