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

void LuxtronikV1Component::update() {
    ESP_LOGD(TAG, "Polling Luxtronik V1 Component...");
    if (this->parent_ != nullptr) {
        this->parent_->write_str("1100\r\n");
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
        if (temperature_vorlauf_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_vorlauf_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Vorlauf: %.1f", value);
        }
        
        // Second value is RL temperature
        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_ruecklauf_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_ruecklauf_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Rücklauf: %.1f", value);
        }

        // Add parsing for additional sensors
        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_ruecklauf_soll_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_ruecklauf_soll_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Rücklauf Soll: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_heissgas_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_heissgas_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Heissgas: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_aussen_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_aussen_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Aussen: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_brauchwasser_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_brauchwasser_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Brauchwasser: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_brauchwasser_soll_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_brauchwasser_soll_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Brauchwasser Soll: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_waermequelle_ein_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_waermequelle_ein_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Wärmequelle Ein: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_kaeltekreis_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_kaeltekreis_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Kältekreis: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_mk1_vorlauf_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_mk1_vorlauf_->publish_state(value);
            ESP_LOGD(TAG, "Temperature MK1 Vorlauf: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_mk1_vorlauf_soll_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_mk1_vorlauf_soll_->publish_state(value);
            ESP_LOGD(TAG, "Temperature MK1 Vorlauf Soll: %.1f", value);
        }

        start = end + 1;
        end = msg.find(delimiter, start);
        if (temperature_raumstat_ != nullptr && end != std::string::npos) {
            std::string temp = msg.substr(start, end - start);
            float value = get_float_temp_(temp);
            temperature_raumstat_->publish_state(value);
            ESP_LOGD(TAG, "Temperature Raumstat: %.1f", value);
        }
    }
}

void LuxtronikV1Component::dump_config() {
    ESP_LOGCONFIG(TAG, "Luxtronik V1 Component:");
    ESP_LOGCONFIG(TAG, "  UART Parent: %s", this->parent_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Vorlauf: %s", this->temperature_vorlauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Rücklauf: %s", this->temperature_ruecklauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Rücklauf Soll: %s", this->temperature_ruecklauf_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Heissgas: %s", this->temperature_heissgas_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Aussen: %s", this->temperature_aussen_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Brauchwasser: %s", this->temperature_brauchwasser_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Brauchwasser Soll: %s", this->temperature_brauchwasser_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Wärmequelle Ein: %s", this->temperature_waermequelle_ein_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Kältekreis: %s", this->temperature_kaeltekreis_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature MK1 Vorlauf: %s", this->temperature_mk1_vorlauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature MK1 Vorlauf Soll: %s", this->temperature_mk1_vorlauf_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Raumstat: %s", this->temperature_raumstat_ ? "Set" : "Not Set");
}

}  // namespace luxtronik_v1_component
}  // namespace esphome