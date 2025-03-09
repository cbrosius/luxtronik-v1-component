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
        parse_temperature_message_(message);
    }
    // Check if it's an input message
    if (msg.find("1200") == 0) {
        parse_input_message_(message);
    }
    // Check if it's an output message
    if (msg.find("1300") == 0) {
        parse_output_message_(message);
    }}

void LuxtronikV1Component::parse_temperature_message_(const char* message) {
    ESP_LOGD(TAG, "Temperature message received: %s", message);
    std::string msg(message);
    
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
    if (temperature_waermequelle_eingang_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = get_float_temp_(temp);
        temperature_waermequelle_eingang_->publish_state(value);
        ESP_LOGD(TAG, "Temperature Wärmequelle Eingang: %.1f", value);
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
    if (temperature_mischkreis1_vorlauf_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = get_float_temp_(temp);
        temperature_mischkreis1_vorlauf_->publish_state(value);
        ESP_LOGD(TAG, "Temperature Mischkreis1 Vorlauf: %.1f", value);
    }

    start = end + 1;
    end = msg.find(delimiter, start);
    if (temperature_mischkreis1_vorlauf_soll_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = get_float_temp_(temp);
        temperature_mischkreis1_vorlauf_soll_->publish_state(value);
        ESP_LOGD(TAG, "Temperature Mischkreis1 Vorlauf Soll: %.1f", value);
    }

    start = end + 1;
    end = msg.find(delimiter, start);
    if (temperature_raumstation_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = get_float_temp_(temp);
        temperature_raumstation_->publish_state(value);
        ESP_LOGD(TAG, "Temperature Raumstation: %.1f", value);
    }

    // Request input values after temperature values are parsed
    this->parent_->write_str("1200\r\n");

}

void LuxtronikV1Component::parse_input_message_(const char* message) {
    ESP_LOGD(TAG, "Input message received: %s", message);
    std::string msg(message);
    
    // Split message by semicolon
    std::string delimiter = ";";
    size_t start = 5;  // Skip "1200;"
    size_t end = msg.find(delimiter, start);
    
    // Skip count
    start = end + 1;
    end = msg.find(delimiter, start);
    
    // First value is Eingang Abtau Soledruck Durchfluss
    if (eingang_abtau_soledruck_durchfluss_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = std::atof(temp.c_str());  // Don't divide by 10 for input values
        eingang_abtau_soledruck_durchfluss_->publish_state(value);
        ESP_LOGD(TAG, "Eingang Abtau Soledruck Durchfluss: %.0f", value);
    }
    
    // Second value is Eingang Sperrzeit EVU
    start = end + 1;
    end = msg.find(delimiter, start);
    if (eingang_sperrzeit_evu_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = std::atof(temp.c_str());  // Don't divide by 10 for input values
        eingang_sperrzeit_evu_->publish_state(value);
        ESP_LOGD(TAG, "Eingang Sperrzeit EVU: %.0f", value);
    }

    // Add parsing for additional input sensors
    start = end + 1;
    end = msg.find(delimiter, start);
    if (eingang_hochdruckpressostat_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = std::atof(temp.c_str());  // Don't divide by 10 for input values
        eingang_hochdruckpressostat_->publish_state(value);
        ESP_LOGD(TAG, "Eingang Hochdruckpressostat: %.0f", value);
    }

    start = end + 1;
    end = msg.find(delimiter, start);
    if (eingang_motorschutz_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = std::atof(temp.c_str());  // Don't divide by 10 for input values
        eingang_motorschutz_->publish_state(value);
        ESP_LOGD(TAG, "Eingang Motorschutz: %.0f", value);
    }

    start = end + 1;
    end = msg.find(delimiter, start);
    if (eingang_niederdruckpressostat_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = std::atof(temp.c_str());  // Don't divide by 10 for input values
        eingang_niederdruckpressostat_->publish_state(value);
        ESP_LOGD(TAG, "Eingang Niederdruckpressostat: %.0f", value);
    }

    start = end + 1;
    end = msg.find(delimiter, start);
    if (eingang_fremdstromanode_ != nullptr && end != std::string::npos) {
        std::string temp = msg.substr(start, end - start);
        float value = std::atof(temp.c_str());  // Don't divide by 10 for input values
        eingang_fremdstromanode_->publish_state(value);
        ESP_LOGD(TAG, "Eingang Fremdstromanode: %.0f", value);
    }

    // Request output values after input values are parsed
    this->parent_->write_str("1300\r\n");
}

void LuxtronikV1Component::parse_output_message_(const char* message) {
    ESP_LOGD(TAG, "Output message received: %s", message);
    std::string msg(message);
    
    // Split message by semicolon
    std::string delimiter = ";";
    size_t start = 5;  // Skip "1300;"
    size_t end = msg.find(delimiter, start);
    
    // Skip count
    start = end + 1;
    end = msg.find(delimiter, start);
    
    // Parse each output value
    auto parse_and_publish = [&](sensor::Sensor* sensor, const char* name) {
        if (sensor != nullptr && end != std::string::npos) {
            std::string value_str = msg.substr(start, end - start);
            float value = std::atof(value_str.c_str());
            sensor->publish_state(value);
            ESP_LOGD(TAG, "Output %s: %.0f", name, value);
        }
        start = end + 1;
        end = msg.find(delimiter, start);
    };

    parse_and_publish(ausgang_abtauventil_, "Abtauventil");
    parse_and_publish(ausgang_bwp_, "BWP");
    parse_and_publish(ausgang_fbhp_, "FBHP");
    parse_and_publish(ausgang_hzp_, "HZP");
    parse_and_publish(ausgang_mischer_1_auf_, "Mischer 1 Auf");
    parse_and_publish(ausgang_mischer_1_zu_, "Mischer 1 Zu");
    parse_and_publish(ausgang_vent_wp_, "Vent WP");
    parse_and_publish(ausgang_vent_brunnen_, "Vent Brunnen");
    parse_and_publish(ausgang_verdichter_1_, "Verdichter 1");
    parse_and_publish(ausgang_verdichter_2_, "Verdichter 2");
    parse_and_publish(ausgang_zpumpe_, "ZPumpe");
    parse_and_publish(ausgang_zwe_, "ZWE");
    parse_and_publish(ausgang_zwe_stoerung_, "ZWE Störung");
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
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Wärmequelle Eingang: %s", this->temperature_waermequelle_eingang_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Kältekreis: %s", this->temperature_kaeltekreis_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Mischkreis1 Vorlauf: %s", this->temperature_mischkreis1_vorlauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Mischkreis1 Vorlauf Soll: %s", this->temperature_mischkreis1_vorlauf_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperature Raumstation: %s", this->temperature_raumstation_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Abtau Soledruck Durchfluss: %s", this->eingang_abtau_soledruck_durchfluss_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Sperrzeit EVU: %s", this->eingang_sperrzeit_evu_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Hochdruckpressostat: %s", this->eingang_hochdruckpressostat_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Motorschutz: %s", this->eingang_motorschutz_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Niederdruckpressostat: %s", this->eingang_niederdruckpressostat_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Fremdstromanode: %s", this->eingang_fremdstromanode_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Abtauventil: %s", this->ausgang_abtauventil_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang BWP: %s", this->ausgang_bwp_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang FBHP: %s", this->ausgang_fbhp_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang HZP: %s", this->ausgang_hzp_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Mischer 1 Auf: %s", this->ausgang_mischer_1_auf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Mischer 1 Zu: %s", this->ausgang_mischer_1_zu_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Vent WP: %s", this->ausgang_vent_wp_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Vent Brunnen: %s", this->ausgang_vent_brunnen_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Verdichter 1: %s", this->ausgang_verdichter_1_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Verdichter 2: %s", this->ausgang_verdichter_2_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang ZPumpe: %s", this->ausgang_zpumpe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang ZWE: %s", this->ausgang_zwe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang ZWE Störung: %s", this->ausgang_zwe_stoerung_ ? "Set" : "Not Set");
}

}  // namespace luxtronik_v1_component
}  // namespace esphome