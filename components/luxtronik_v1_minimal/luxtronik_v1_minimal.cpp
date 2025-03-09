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
    if (this->parent_ == nullptr) {
        ESP_LOGW(TAG, "Cannot loop - UART parent not set");
        return;
    }
    
    while (this->parent_->available()) {
        char c = this->parent_->read();
        if (c == ASCII_CR || c == ASCII_LF) {
            if (this->read_pos_ > 0) {
                this->read_buffer_[this->read_pos_] = '\0';
                ESP_LOGD(TAG, "Received: %s", this->read_buffer_);
                this->read_pos_ = 0;
            }
        } else {
            this->read_buffer_[this->read_pos_++] = c;
            if (this->read_pos_ >= READ_BUFFER_LENGTH) {
                this->read_pos_ = 0;
            }
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