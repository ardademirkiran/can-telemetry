#ifndef CANCLIENT_HPP
#define CANCLIENT_HPP

#include "CANN_COMM_STATUS.hpp"
#include "OBDResponse.hpp"
#include <cstdint>
extern "C"
{
#include "driver/twai.h"
}
#include "array"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include <string>

class CANClient
{
public:
  CANClient();
  ~CANClient();

  twai_message_t last_tx_msg;

  void setup_twai();
  bool hasTraffic();
  OBDResponse send_command(uint8_t mode, uint8_t pid, uint32_t ecuId);
  CanCommStatus send_frame(twai_message_t &last_tx_msg);

  CanCommStatus receive_frame(uint32_t ecu_id, twai_message_t &rx_msg, uint8_t pid);
  CanCommStatus handle_consecutive_frames(std::vector<uint8_t> &payload_vector, uint16_t response_length, uint16_t received_bytes);
  void build_tx_message(twai_message_t &tx_msg, uint8_t pci, uint8_t mode, uint8_t pid, uint32_t ecu_id);
  OBDResponse handle_error(CanCommStatus error_status);
};

#endif // CANCLIENT_HPP
