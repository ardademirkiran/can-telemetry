#ifndef _NVS_INTERFACE_HPP_
#define _NFS_INTERFACE_HPP_

#include <cstdint>
#include <string>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <memory>
class NVSInterface
{
public:
    void init_nvs_interface();
    void deinit_nvs_interface();
    int32_t read_int_data(const std::string &key);
    std::string read_string_data(const std::string &key);
    void write_data(const std::string &key, const std::string &data);
    void write_data(const std::string &key, int32_t data);

private:
    void write_int_data();
    void write_string_data();
    const char *TAG = "NVS_INTF";
    nvs_handle_t nvs_handle;
};

#endif //_NFS_INTERFACE_HPP