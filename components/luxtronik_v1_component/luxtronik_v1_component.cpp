#include "esphome/core/log.h"
#include "luxtronik_v1_component.h"

namespace esphome {
namespace luxtronik_v1_component {

static const char *TAG = "luxtronik_v1_component.component";

void LuxtronikV1Component::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Luxtronik V1 Component...");
    if (this->parent_ == nullptr) {
        ESP_LOGE(TAG, "UART parent not set!");
        this->mark_failed();
        return;
    }
}

void LuxtronikV1Component::loop() {
    if (this->parent_ == nullptr) {
        ESP_LOGW(TAG, "Cannot loop - UART parent not set");
        return;
    }
    
    while (this->parent_->available()) {
        uint8_t c;
        if (!this->parent_->read_byte(&c)) {
            continue;
        }
        
        if (c == ASCII_CR || c == ASCII_LF) {
            if (this->read_pos_ > 0) {
                this->read_buffer_[this->read_pos_] = '\0';
                ESP_LOGD(TAG, "Received: %s", this->read_buffer_);
                parse_message_(this->read_buffer_);
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

void LuxtronikV1Component::parse_message_(const char* message) {
    std::string msg(message);
    
    // Check if it's a temperature message
    if (msg.find("1100") == 0) {
        ESP_LOGD(TAG, "Temperature message received: %s", message);
        
        // Split message by semicolon
        std::string delimiter = ";";
        size_t start = 5;  // Skip "1100;"
        size_t end = msg.find(delimiter, start);
        
        // Skip count
        start = end + 1;
        end = msg.find(delimiter, start);
        
        // First value is VL temperature
        if (temp_vl_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temp_vl_->publish_state(value);
            ESP_LOGD(TAG, "VL Temperature: %.1f", value);
        }
        
        // Second value is RL temperature
        start = end + 1;
        end = msg.find(delimiter, start);
        if (temp_rl_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temp_rl_->publish_state(value);
            ESP_LOGD(TAG, "RL Temperature: %.1f", value);
        }
    }
}

void LuxtronikV1Component::dump_config() {
    ESP_LOGCONFIG(TAG, "Luxtronik V1 Component:");
    ESP_LOGCONFIG(TAG, "  UART Parent: %s", this->parent_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  VL Sensor: %s", this->temp_vl_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  RL Sensor: %s", this->temp_rl_ ? "Not Set": "Not Set");
}

}  // namespace luxtronik_v1_component
}  // namespace esphome