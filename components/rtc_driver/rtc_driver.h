#pragma once

#include <string.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t date;
    uint8_t month;
    uint8_t dow;
    uint8_t year;
} time_dto;

void rtc_initialize_gpio(void);
void rtc_sync_device_time(void);
void rtc_sync_clock_time(time_dto time_dto);

#ifdef __cplusplus
}
#endif
