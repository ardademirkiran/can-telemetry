extern "C"
{
#include "driver/twai.h"
}
#include "esp_log.h"
#include "esp_timer.h"
#include "array"
#include "OBDResponse.hpp"
#include "CANClient.hpp"

using namespace std;

static const char *TAG = "CAN_MODULE";
CANClient::CANClient() {}
CANClient::~CANClient()
{
    twai_stop();
    twai_driver_uninstall();
}

void CANClient::setup_twai()
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Start TWAI driver
    err = twai_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "TWAI started successfully (500kbps, RX=GPIO%d, TX=GPIO%d)", GPIO_NUM_22, GPIO_NUM_21);
}

bool CANClient::hasTraffic()
{
    OBDResponse response = sendCommand(0x00, 0x7E8);
    return (response.getStatus() == OBDResponseStatus::OBD_OK);
}

OBDResponse CANClient::sendCommand(uint8_t pid, uint32_t ecuId)
{
    uint8_t messageData[8];
    OBDResponse response = OBDResponse();
    memset(messageData, 0x00, sizeof(messageData));

    twai_message_t canMessage = {};
    canMessage.identifier = 0x7DF;
    canMessage.extd = 0;
    canMessage.rtr = 0;
    canMessage.ss = 0;
    canMessage.self = 0;
    canMessage.data_length_code = 8;
    messageData[0] = 0x02;
    messageData[1] = 0x01;
    messageData[2] = pid;
    memcpy(canMessage.data, messageData, sizeof(messageData));

    esp_err_t err = twai_transmit(&canMessage, pdMS_TO_TICKS(1000));

    if (err != ESP_OK)
    {
        response.setStatus(OBDResponseStatus::OBD_ERROR);
        return response;
    }
    else
    {
        int64_t start = esp_timer_get_time() / 1000; // ms
        while ((esp_timer_get_time() / 1000 - start) < 2000)
        {
            twai_message_t rx_msg;
            if (twai_receive(&rx_msg, pdMS_TO_TICKS(200)) == ESP_OK)
            {
                if (rx_msg.identifier == ecuId && rx_msg.data[1] == 0x41 && rx_msg.data[2] == pid)
                {
                    response.setData(rx_msg.data, rx_msg.data_length_code);
                    response.setId(rx_msg.identifier);
                    response.setPid(rx_msg.data[2]);
                    response.setStatus(OBDResponseStatus::OBD_OK);
                    return response;
                }
            }
            else
            {
                response.setStatus(OBDResponseStatus::OBD_ERROR);
                return response;
            }
        }

        response.setStatus(OBDResponseStatus::OBD_TIMEOUT);
        return response;
    }
}
