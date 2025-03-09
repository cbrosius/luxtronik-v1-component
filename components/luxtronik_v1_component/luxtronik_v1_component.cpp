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
    
    if (msg.empty() || msg.length() < 4) {
        return;
    }

    // Use substring comparison for better performance
    std::string prefix = msg.substr(0, 4);
    
    if (prefix == "1100") {
        this->defer([this, msg]() {
            parse_temperature_message_(msg.c_str());
        });
    } else if (prefix == "1200") {
        this->defer([this, msg]() {
            parse_input_message_(msg.c_str());
        });
    } else if (prefix == "1300") {
        this->defer([this, msg]() {
            parse_output_message_(msg.c_str());
        });
    }
}

void LuxtronikV1Component::parse_temperature_message_(const char* message) {
    std::string msg(message);
    std::vector<std::string> values;
    size_t start = 5;  // Skip "1100;"
    size_t end = 0;
    
    // Split message into vector for faster processing
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
    auto publish_temp = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float temp = get_float_temp_(value);
            sensor->publish_state(temp);
            // ESP_LOGD(TAG, "Temperature %s: %.1f", name, temp);
        }
    };

    // Process all temperature sensors
    if (idx < values.size()) publish_temp(temperature_vorlauf_, values[idx++], "Vorlauf");
    if (idx < values.size()) publish_temp(temperature_ruecklauf_, values[idx++], "Rücklauf");
    if (idx < values.size()) publish_temp(temperature_ruecklauf_soll_, values[idx++], "Rücklauf Soll");
    if (idx < values.size()) publish_temp(temperature_heissgas_, values[idx++], "Heissgas");
    if (idx < values.size()) publish_temp(temperature_aussen_, values[idx++], "Aussen");
    if (idx < values.size()) publish_temp(temperature_brauchwasser_, values[idx++], "Brauchwasser");
    if (idx < values.size()) publish_temp(temperature_brauchwasser_soll_, values[idx++], "Brauchwasser Soll");
    if (idx < values.size()) publish_temp(temperature_waermequelle_eingang_, values[idx++], "Wärmequelle Eingang");
    if (idx < values.size()) publish_temp(temperature_kaeltekreis_, values[idx++], "Kältekreis");
    if (idx < values.size()) publish_temp(temperature_mischkreis1_vorlauf_, values[idx++], "Mischkreis1 Vorlauf");
    if (idx < values.size()) publish_temp(temperature_mischkreis1_vorlauf_soll_, values[idx++], "Mischkreis1 Vorlauf Soll");
    if (idx < values.size()) publish_temp(temperature_raumstation_, values[idx++], "Raumstation");

    // Request input values after temperature values are parsed
    this->parent_->write_str("1200\r\n");
}

void LuxtronikV1Component::parse_input_message_(const char* message) {
    ESP_LOGD(TAG, "Input message received: %s", message);
    std::string msg(message);
    std::vector<std::string> values;
    size_t start = 5;  // Skip "1200;"
    size_t end = 0;
    
    // Split message into vector for faster processing
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
    auto publish_input = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            sensor->publish_state(val);
            // ESP_LOGD(TAG, "Input %s: %.0f", name, val);
        }
    };

    // Process all input sensors
    if (idx < values.size()) publish_input(eingang_abtau_soledruck_durchfluss_, values[idx++], "Abtau Soledruck Durchfluss");
    if (idx < values.size()) publish_input(eingang_sperrzeit_evu_, values[idx++], "Sperrzeit EVU");
    if (idx < values.size()) publish_input(eingang_hochdruckpressostat_, values[idx++], "Hochdruckpressostat");
    if (idx < values.size()) publish_input(eingang_motorschutz_, values[idx++], "Motorschutz");
    if (idx < values.size()) publish_input(eingang_niederdruckpressostat_, values[idx++], "Niederdruckpressostat");
    if (idx < values.size()) publish_input(eingang_fremdstromanode_, values[idx++], "Fremdstromanode");

    // Request output values after input values are parsed
    this->parent_->write_str("1300\r\n");
}

void LuxtronikV1Component::parse_output_message_(const char* message) {
    ESP_LOGD(TAG, "Output message received: %s", message);
    std::string msg(message);
    std::vector<std::string> values;
    size_t start = 5;  // Skip "1300;"
    size_t end = 0;
    
    // Split message into vector for faster processing
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
    auto publish_output = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            sensor->publish_state(val);
            // ESP_LOGD(TAG, "Output %s: %.0f", name, val);
        }
    };

    // Process all output sensors
    if (idx < values.size()) publish_output(ausgang_abtauventil_, values[idx++], "Abtauventil");
    if (idx < values.size()) publish_output(ausgang_bwp_, values[idx++], "BWP");
    if (idx < values.size()) publish_output(ausgang_fbhp_, values[idx++], "FBHP");
    if (idx < values.size()) publish_output(ausgang_hzp_, values[idx++], "HZP");
    if (idx < values.size()) publish_output(ausgang_mischer_1_auf_, values[idx++], "Mischer 1 Auf");
    if (idx < values.size()) publish_output(ausgang_mischer_1_zu_, values[idx++], "Mischer 1 Zu");
    if (idx < values.size()) publish_output(ausgang_vent_wp_, values[idx++], "Vent WP");
    if (idx < values.size()) publish_output(ausgang_vent_brunnen_, values[idx++], "Vent Brunnen");
    if (idx < values.size()) publish_output(ausgang_verdichter_1_, values[idx++], "Verdichter 1");
    if (idx < values.size()) publish_output(ausgang_verdichter_2_, values[idx++], "Verdichter 2");
    if (idx < values.size()) publish_output(ausgang_zpumpe_, values[idx++], "ZPumpe");
    if (idx < values.size()) publish_output(ausgang_zwe_, values[idx++], "ZWE");
    if (idx < values.size()) publish_output(ausgang_zwe_stoerung_, values[idx++], "ZWE Störung");
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