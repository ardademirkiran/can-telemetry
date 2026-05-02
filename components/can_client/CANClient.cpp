
#include "CANClient.hpp"

#include <iomanip>
#include <sstream>

using namespace std;

static const char *TAG = "CAN_MODULE";
CANClient::CANClient() {}
CANClient::~CANClient()
{
    twai_stop();
    twai_driver_uninstall();
}

void log_twai_message(twai_message_t &msg)
{
    std::ostringstream oss;

    for (int i = 0; i < msg.data_length_code; i++)
    {
        oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(msg.data[i])
            << " ";
    }

    ESP_LOGI("RECEIVED_FRAME", "%s", oss.str().c_str());
}

OBDResponse CANClient::handle_error(CanCommStatus error_status)
{
    ESP_LOGI(TAG, "Error status.");
    OBDResponse error_response{};
    switch (error_status)
    {
    case CanCommStatus::TWAI_NOT_INSTALLED:
        this->setup_twai();
        break;
    case CanCommStatus::OUT_OF_SEQUENCE:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status OOS.");
        break;
    case CanCommStatus::RECEIVE_FAIL:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status RECEIVE_FAIL.");
        break;
    case CanCommStatus::RECEIVE_TIMEOUT:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status RECEIVE_TIMEOUT.");
        break;
    case CanCommStatus::RECEIVE_INVALID_ARG:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status RECEIVE_INVALID_ARG.");

        break;
    case CanCommStatus::SEND_INVALID_ARG:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status SEND_INVALID_ARG.");

        break;
    case CanCommStatus::SEND_TIMEOUT:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status SEND_TIMEOUT.");

        break;
    case CanCommStatus::SEND_TX_BUSY:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status SEND_TX_BUSY.");

        break;
    case CanCommStatus::SEND_FAIL:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
        ESP_LOGI(TAG, "Error status SEND_FAIL.");

        break;
    default:
        error_response.setStatus(OBDResponseStatus::OBD_ERROR);
    }

    return error_response;
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

CanCommStatus CANClient::handle_consecutive_frames(std::vector<uint8_t> &payload_vector, uint16_t response_length, uint16_t received_bytes)
{

    // multiple frames

    ESP_LOGI(TAG, "Sending flow control...");

    twai_message_t tx_msg;
    build_tx_message(tx_msg, 0x30, 0x00, 0x00, 0x7DF);

    CanCommStatus send_status = send_frame(tx_msg);

    if (send_status != CanCommStatus::SUCCESS)
    {
        return send_status;
    }

    uint8_t sequence_num = 0;

    while (received_bytes < response_length)
    {
        ESP_LOGI(TAG, "Consecutive frame fetch phase...");

        // consecutive frames
        uint16_t remaining = response_length - received_bytes;
        uint8_t chunk_size = (remaining < 7) ? remaining : 7;

        twai_message_t rx_msg;

        CanCommStatus receive_status = receive_frame(0x7E8, rx_msg, 0xFF);

        if (receive_status == CanCommStatus::SUCCESS)
        {
            if ((rx_msg.data[0] >> 4) == 0x2)
            {
                uint8_t sequence_num_input = rx_msg.data[0] & 0x0F;

                if (sequence_num_input != sequence_num + 1)
                {
                    // Out of sequence;
                    return CanCommStatus::OUT_OF_SEQUENCE;
                }

                sequence_num++;
                payload_vector.insert(payload_vector.end(), std::begin(rx_msg.data) + 1, std::begin(rx_msg.data) + 1 + chunk_size);
                received_bytes += chunk_size;
            }
            else
            {
                return CanCommStatus::RECEIVE_FAIL;
            }
        }
        else
        {
            return receive_status;
        }
    }
    return CanCommStatus::SUCCESS;
}

OBDResponse CANClient::send_command(uint8_t mode, uint8_t pid, uint32_t ecuId)
{
    ESP_LOGI(TAG, "Sending command: ECU: %x \t mode: %x \t PID:%x", ecuId, mode, pid);

    twai_message_t tx_msg;
    build_tx_message(tx_msg, 0x02, mode, pid, 0x7DF);

    CanCommStatus sent_status = send_frame(tx_msg);

    if (sent_status != CanCommStatus::SUCCESS)
    {
        return this->handle_error(sent_status);
    }

    ESP_LOGI(TAG, "Send is successfull.");
    twai_message_t rx_msg;

    CanCommStatus receive_status = receive_frame(0x7E8, rx_msg, pid);

    if (receive_status != CanCommStatus::SUCCESS)
    {
        return this->handle_error(receive_status);
    }

    std::vector<uint8_t> payload_vector;
    uint8_t pci = rx_msg.data[0];
    uint8_t type = (pci >> 4);

    if (type == 0x0)
    {
        // single frame
        ESP_LOGI(TAG, "-Single Frame-");

        uint16_t response_length = (pci & 0x0F);
        payload_vector = std::vector<uint8_t>{&rx_msg.data[1], &rx_msg.data[1] + response_length};
        return OBDResponse{OBDResponseStatus::OBD_OK, ecuId, pid, payload_vector, response_length};
    }
    else if (type == 0x1)
    {
        // multiple frames
        ESP_LOGI(TAG, "-Multi Frame-");

        uint16_t response_length = ((uint16_t)(pci & 0x0F) << 8) | rx_msg.data[1];
        uint16_t received_bytes = 6;
        payload_vector = std::vector<uint8_t>{&rx_msg.data[2], &rx_msg.data[2] + 6};

        CanCommStatus sequence_status = handle_consecutive_frames(payload_vector, response_length, received_bytes);

        if (sequence_status == CanCommStatus::SUCCESS)
        {
            return OBDResponse{OBDResponseStatus::OBD_OK, ecuId, pid, payload_vector, response_length};
        }
        else
        {
            return this->handle_error(sequence_status);
        }
    }
    else
    {
        return this->handle_error(CanCommStatus::RECEIVE_FAIL);
    }
}

void CANClient::build_tx_message(twai_message_t &tx_msg, uint8_t pci, uint8_t mode, uint8_t pid, uint32_t ecu_id)
{
    uint8_t messageData[8];
    memset(messageData, 0x00, sizeof(messageData));

    tx_msg.identifier = ecu_id;
    tx_msg.extd = 0;
    tx_msg.rtr = 0;
    tx_msg.ss = 0;
    tx_msg.self = 0;
    tx_msg.data_length_code = 8;
    messageData[0] = pci;
    messageData[1] = mode;
    messageData[2] = pid;
    memcpy(tx_msg.data, messageData, sizeof(messageData));
}

CanCommStatus CANClient::send_frame(twai_message_t &tx_msg)
{

    esp_err_t send_status = twai_transmit(&tx_msg, pdMS_TO_TICKS(10000));

    switch (send_status)
    {
    case ESP_OK:
        return CanCommStatus::SUCCESS;
    case ESP_ERR_INVALID_ARG:
        return CanCommStatus::SEND_INVALID_ARG;
    case ESP_ERR_TIMEOUT:
        return CanCommStatus::SEND_TIMEOUT;
    case ESP_FAIL:
        return CanCommStatus::SEND_TX_BUSY;
    case ESP_ERR_INVALID_STATE:
        return CanCommStatus::TWAI_NOT_INSTALLED;
    default:
        return CanCommStatus::SEND_FAIL;
    }
}

CanCommStatus CANClient::receive_frame(uint32_t ecu_id, twai_message_t &rx_msg, uint8_t pid)
{
    int64_t start = esp_timer_get_time() / 1000; // ms
    while ((esp_timer_get_time() / 1000 - start) < 5000)
    {
        esp_err_t receive_status = twai_receive(&rx_msg, pdMS_TO_TICKS(500));
        switch (receive_status)
        {
        case ESP_OK:
            if (rx_msg.identifier == ecu_id && (rx_msg.data[2] == pid || rx_msg.data[2] == 0xFF))
            {
                log_twai_message(rx_msg);
                return CanCommStatus::SUCCESS;
            }
            break;
        case ESP_ERR_INVALID_ARG:
            return CanCommStatus::RECEIVE_INVALID_ARG; // argıments are invalid
                                                       // check rx_msg structure
                                                       // here
        case ESP_ERR_INVALID_STATE:
            return CanCommStatus::TWAI_NOT_INSTALLED; // twai not installed
                                                      // retry twai install
        case ESP_ERR_TIMEOUT:
            return CanCommStatus::RECEIVE_TIMEOUT; // timed out waiting for
                                                   // message
        default:
            return CanCommStatus::RECEIVE_FAIL; // generic fail, retry
        }
    }

    return CanCommStatus::RECEIVE_TIMEOUT;
}

bool CANClient::hasTraffic()
{
    OBDResponse response = this->send_command(0x01, 0x0C, 0x7E0);

    if (response.getStatus() == OBDResponseStatus::OBD_OK)
    {
        return true;
    }

    return false;
}
