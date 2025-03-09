#include "esphome/core/log.h"
#include "luxtronik_v1_minimal.h"

namespace esphome {
namespace luxtronik_v1_minimal {

static const char *TAG = "luxtronik_v1_minimal.sensor";

void LuxtronikV1MinimalSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Luxtronik V1 Minimal...");
    if (this->parent_ == nullptr) {
        ESP_LOGE(TAG, "UART parent not set!");
        this->mark_failed();
        return;
    }
}

void LuxtronikV1MinimalSensor::update() {
        send_cmd("1100");
}

void LuxtronikV1MinimalSensor::loop() {
    // Read message
    while (this->available()) {
        uint8_t byte;
        this->read_byte(&byte);

        if (this->read_pos_ == READ_BUFFER_LENGTH)
        this->read_pos_ = 0;

        ESP_LOGV(TAG, "Buffer pos: %u %d", this->read_pos_, byte);  // NOLINT

        if (byte == ASCII_CR)
        continue;
        if (byte >= 0x7F)
        byte = '?';  // need to be valid utf8 string for log functions.
        this->read_buffer_[this->read_pos_] = byte;

        if (this->read_buffer_[this->read_pos_] == ASCII_LF) {
        this->read_buffer_[this->read_pos_] = 0;
        this->read_pos_ = 0;
        this->parse_cmd_(this->read_buffer_);
        } else {
        this->read_pos_++;
        }
    }
}

void LuxtronikV1MinimalSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "LuxtronikV1MinimalSensor:");
    ESP_LOGCONFIG(TAG, "  UART Parent: %s", this->parent_ ? "Set" : "Not Set");
}

void LuxtronikV1MinimalSensor::send_cmd(std::string message) {
    if (this->parent_ == nullptr) {
        ESP_LOGW(TAG, "Cannot send command - UART parent not set");
        return;
    }
    
    ESP_LOGV(TAG, "Sending command: %s", message.c_str());
    message += "\r\n";
    this->parent_->write_str(message.c_str());
}


}  // namespace luxtronik_v1_minimal
}  // namespace esphome