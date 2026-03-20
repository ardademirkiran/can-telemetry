#include "nvs_interface.hpp"

void NVSInterface::init_nvs_interface()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "NVS Interface is initialized.");
}

void NVSInterface::deinit_nvs_interface()
{
    esp_err_t err = nvs_flash_deinit();
    if (err == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        return;
    }
}

void NVSInterface::write_data(const std::string &key, const std::string &data)
{
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    nvs_set_str(nvs_handle, key.c_str(), data.c_str());
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "Data write - Key=%s Value=%s", key.c_str(), data.c_str());
}

void NVSInterface::write_data(const std::string &key, int32_t data)
{
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    nvs_set_i32(nvs_handle, key.c_str(), data);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Data write - Key=%s Value=%d", key.c_str(), data);
}

int32_t NVSInterface::read_int_data(const std::string &key)
{
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    int32_t data_read = 0;
    nvs_get_i32(nvs_handle, key.c_str(), &data_read);
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Data Read - Key=%s Value=%d", key.c_str(), data_read);
    return data_read;
}

std::string NVSInterface::read_string_data(const std::string &key)
{
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    std::string data_read;
    size_t size = 0;
    nvs_get_str(nvs_handle, key.c_str(), NULL, &size);
    data_read.resize(size);
    nvs_get_str(nvs_handle, key.c_str(), data_read.data(), &size);
    if (!data_read.empty())
    {
        data_read.pop_back();
    }
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Data Read - Key=%s Value=%s", key.c_str(), data_read.c_str());

    return data_read;
}