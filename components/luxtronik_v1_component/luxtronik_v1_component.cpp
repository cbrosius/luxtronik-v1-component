#include "esphome/core/log.h"
#include "luxtronik_v1_component.h"

#include <vector> // Required for std::vector
#include <string> // Required for std::string

namespace esphome {
namespace luxtronik_v1_component {

static const char *TAG = "luxtronik_v1_component.component";

// Protocol Command Constants
static const char* CMD_GET_TEMPERATURES = "1100";
static const char* CMD_GET_INPUTS = "1200";
static const char* CMD_GET_OUTPUTS = "1300";
static const char* CMD_GET_HEATING_MODE = "3405";
static const char* CMD_GET_HOT_WATER_MODE = "3505";
static const char* CMD_GET_STATUS = "1700";
static const char* CMD_GET_ERRORS_BASE = "1500"; // Base prefix for error messages
static const char* CMD_GET_OPERATING_HOURS = "1450";
static const char* CMD_GET_HEATING_CURVE = "3400";
static const char* CMD_PROGRAMMING_ERROR_PREFIX = "779";
static const char* CMD_SAVE_PROGRAMMING = "999";
static const char* CMD_PROGRAM_HOT_WATER_MODE = "3506";
static const char* CMD_PROGRAM_HEATING_MODE = "3406";
static const char* CMD_PROGRAM_HOT_WATER_TEMP = "3507";

// Implementation of the new helper function
bool LuxtronikV1Component::split_message_(const std::string& msg, std::vector<std::string>& values, const std::string& expected_prefix) {
    values.clear();
    if (msg.rfind(expected_prefix, 0) != 0) { // Check if msg starts with expected_prefix
        ESP_LOGW(TAG, "Message '%s' does not start with expected prefix '%s'", msg.c_str(), expected_prefix.c_str());
        return false;
    }

    size_t prefix_len = expected_prefix.length();
    if (msg.length() <= prefix_len || msg[prefix_len] != ';') {
        ESP_LOGW(TAG, "Message '%s' has invalid format after prefix '%s'", msg.c_str(), expected_prefix.c_str());
        return false;
    }

    size_t start = prefix_len + 1; // Skip "<prefix>;"
    size_t end = 0;
    
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    if (start < msg.length()) { // Add the last part
        values.push_back(msg.substr(start));
    }
    
    if (values.empty() || values.size() < 1) { // Original code often expects "count" as first value, so at least 1 value (the count itself)
        ESP_LOGW(TAG, "Message '%s' resulted in no values after splitting.", msg.c_str());
        return false; 
    }
    // It's common for the first value to be a count of subsequent values.
    // The original code checks "if (values.size() < 2) return;" which means count + at least one data value.
    // We'll keep this check more generic here, specific parsers can check values.size() further.
    return true;
}

void LuxtronikV1Component::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Luxtronik V1 Component...");
    if (this->parent_ == nullptr) {
        ESP_LOGE(TAG, "UART parent not set!");
        this->mark_failed();
        return;
    }

    // Request initial values immediately after setup
    this->parent_->write_str(std::string(CMD_GET_TEMPERATURES) + "\r\n");
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
        this->parent_->write_str(std::string(CMD_GET_TEMPERATURES) + "\r\n");
    }
}

// Replace the existing publish_state_deferred_ implementation:
void LuxtronikV1Component::publish_state_deferred_(sensor::Sensor* sensor, float new_value, const char* type, const char* name) {
    if (sensor == nullptr) return;

    // Get current state and compare with new value
    float current = sensor->state;
    if (std::isnan(current) || current != new_value) {
        this->defer([this, sensor, new_value, type, name]() {
            sensor->publish_state(new_value);
            ESP_LOGV(TAG, "%s %s: %.1f", type, name, new_value);
        });
    }
}

// Add a helper method for text sensors
void LuxtronikV1Component::publish_text_state_deferred_(text_sensor::TextSensor* sensor, const std::string& new_value, const char* type, const char* name) {
    if (sensor == nullptr) return;

    // Get current state and compare with new value
    std::string current = sensor->get_state();
    if (current.empty() || current.compare(new_value) != 0) {
        this->defer([this, sensor, new_value, type, name]() {
            sensor->publish_state(new_value);
            ESP_LOGV(TAG, "%s %s: changed from '%s' to '%s'", 
                     type, name, sensor->get_state().c_str(), new_value.c_str());
        });
    }
}

void LuxtronikV1Component::publish_timestamp_state_deferred_(text_sensor::TextSensor* sensor, const char* buffer, const char* type, const char* name) {
    if (sensor == nullptr) return;

    std::string new_value = buffer;
    std::string current = sensor->get_state();
    if (current.empty() || current.compare(new_value) != 0) {
        this->defer([this, sensor, new_value, type, name]() {
            sensor->publish_state(new_value);
            ESP_LOGV(TAG, "%s %s: changed from '%s' to '%s'", 
                     type, name, sensor->get_state().c_str(), new_value.c_str());
        });
    }
}

void LuxtronikV1Component::parse_message_(const char* message) {
    std::string msg(message);
    
    if (msg.empty() || msg.length() < 4) {
        return;
    }

    // Get prefix up to first semicolon
    size_t semicolon_pos = msg.find(';');
    if (semicolon_pos == std::string::npos) return;
    std::string prefix = msg.substr(0, semicolon_pos);

    if (prefix == CMD_GET_TEMPERATURES) { // temperature message -> get temperature values
        this->defer([this, msg]() {
            parse_temperatur_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_INPUTS) { // input message -> get input values
        this->defer([this, msg]() {
            parse_input_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_OUTPUTS) { // output message -> get output values
        this->defer([this, msg]() {
            parse_output_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_HEATING_MODE) { // modus_heizung -> get modus values
        this->defer([this, msg]() {
            parse_modus_heizung_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_HOT_WATER_MODE) { // modus_brauchwasser -> get modus values
        this->defer([this, msg]() {
            parse_modus_brauchwasser_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_STATUS) { // status message -> get status values
        this->defer([this, msg]() {
            parse_status_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_ERRORS_BASE) { // error message -> get error values
        this->defer([this, msg]() {
            parse_error_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_OPERATING_HOURS) { // operating hours -> get operating hours values
        this->defer([this, msg]() {
            parse_operatinghours_message_(msg.c_str());
        });
    } else if (prefix == CMD_GET_HEATING_CURVE) { // heating curve -> get heating curve values
        this->defer([this, msg]() {
            parse_heatingcurve_message_(msg.c_str());
        });
    } else if (prefix == CMD_PROGRAMMING_ERROR_PREFIX) { // programming error -> reset programming mode
        this->defer([this, msg]() {
            reset_programming_mode_(msg.c_str());
        });
    }
}

void LuxtronikV1Component::parse_temperatur_message_(const char* message) {
    // ESP_LOGD(TAG, "Temperatures message received: %s", message);
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_TEMPERATURES)) {
        return;
    }
    // values[0] is the count, data starts from values[1]
    if (values.size() < 2) { // Need at least count and one data value
        ESP_LOGW(TAG, "Temperatures message '%s' has insufficient data after splitting.", msg_str.c_str());
        return;
    }
    
    size_t idx = 1;  // Data starts from index 1, as values[0] is the count
    auto publish_temp = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float temp = get_float_temp_(value);
            publish_state_deferred_(sensor, temp, "Temperatur", name);
    
            // If this is the target water temperature sensor, update the number component if value changed
            if (sensor == temperatur_brauchwasser_soll_ && warmwasser_solltemperatur_number_ != nullptr) {
                float current = warmwasser_solltemperatur_number_->state;
                if (std::isnan(current) || current != temp) {
                    this->defer([this, temp]() {
                        warmwasser_solltemperatur_number_->publish_state(temp);
                        ESP_LOGV(TAG, "Updated Warmwasser Solltemperatur Number to: %.1f°C", temp);
                    });
                }
            }
        }
    };

    // Process all temperature sensors
    if (idx < values.size()) publish_temp(temperatur_vorlauf_, values[idx++], "Vorlauf");
    if (idx < values.size()) publish_temp(temperatur_ruecklauf_, values[idx++], "Rücklauf");
    if (idx < values.size()) publish_temp(temperatur_ruecklauf_soll_, values[idx++], "Rücklauf Soll");
    if (idx < values.size()) publish_temp(temperatur_heissgas_, values[idx++], "Heissgas");
    if (idx < values.size()) publish_temp(temperatur_aussen_, values[idx++], "Aussen");
    if (idx < values.size()) publish_temp(temperatur_brauchwasser_, values[idx++], "Brauchwasser");
    if (idx < values.size()) publish_temp(temperatur_brauchwasser_soll_, values[idx++], "Brauchwasser Soll");
    if (idx < values.size()) publish_temp(temperatur_waermequelle_eingang_, values[idx++], "Wärmequelle Eingang");
    if (idx < values.size()) publish_temp(temperatur_kaeltekreis_, values[idx++], "Kältekreis");
    if (idx < values.size()) publish_temp(temperatur_mischkreis1_vorlauf_, values[idx++], "Mischkreis1 Vorlauf");
    if (idx < values.size()) publish_temp(temperatur_mischkreis1_vorlauf_soll_, values[idx++], "Mischkreis1 Vorlauf Soll");
    if (idx < values.size()) publish_temp(temperatur_raumstation_, values[idx++], "Raumstation");

    // Request input values after temperature values are parsed
    this->parent_->write_str(std::string(CMD_GET_INPUTS) + "\r\n");
}

void LuxtronikV1Component::parse_input_message_(const char* message) {
    // ESP_LOGD(TAG, "Input message received: %s", message);
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_INPUTS)) {
        return;
    }
    if (values.size() < 2) { // Need at least count and one data value
        ESP_LOGW(TAG, "Input message '%s' has insufficient data after splitting.", msg_str.c_str());
        return;
    }
    
    size_t idx = 1;  // Data starts from index 1, as values[0] is the count
    auto publish_input = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            publish_state_deferred_(sensor, val, "Input", name);
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
    this->parent_->write_str(std::string(CMD_GET_OUTPUTS) + "\r\n");
}

void LuxtronikV1Component::parse_output_message_(const char* message) {
    // ESP_LOGD(TAG, "Output message received: %s", message);
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_OUTPUTS)) {
        return;
    }
    if (values.size() < 2) { // Need at least count and one data value
        ESP_LOGW(TAG, "Output message '%s' has insufficient data after splitting.", msg_str.c_str());
        return;
    }
    
    size_t idx = 1;  // Data starts from index 1, as values[0] is the count
    auto publish_output = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            publish_state_deferred_(sensor, val, "Output", name);
        }
    };

    // Process all output sensors
    if (idx < values.size()) publish_output(ausgang_abtauventil_, values[idx++], "Abtauventil");
    if (idx < values.size()) publish_output(ausgang_brauchwasserpumpe_, values[idx++], "BWP");
    if (idx < values.size()) publish_output(ausgang_fussbodenheizungspumpe_, values[idx++], "FBHP");
    if (idx < values.size()) publish_output(ausgang_heizungspumpe_, values[idx++], "HZP");
    if (idx < values.size()) publish_output(ausgang_mischer_1_auf_, values[idx++], "Mischer 1 Auf");
    if (idx < values.size()) publish_output(ausgang_mischer_1_zu_, values[idx++], "Mischer 1 Zu");
    if (idx < values.size()) publish_output(ausgang_ventilator_waermepumpe_, values[idx++], "Vent WP");
    if (idx < values.size()) publish_output(ausgang_ventilator_brunnen_, values[idx++], "Vent Brunnen");
    if (idx < values.size()) publish_output(ausgang_verdichter_1_, values[idx++], "Verdichter 1");
    if (idx < values.size()) publish_output(ausgang_verdichter_2_, values[idx++], "Verdichter 2");
    if (idx < values.size()) publish_output(ausgang_zirkulationspumpe_, values[idx++], "ZPumpe");
    if (idx < values.size()) publish_output(ausgang_zweiter_waermeerzeuger_, values[idx++], "ZWE");
    if (idx < values.size()) publish_output(ausgang_zweiter_waermeerzeuger_stoerung_, values[idx++], "ZWE Störung");

    // Request heizungs-modus after input values are parsed
    this->parent_->write_str(std::string(CMD_GET_HEATING_MODE) + "\r\n");

}

std::string LuxtronikV1Component::get_modus_text_(int state) {
    switch (state) {
        case 0: return "Automatik";
        case 1: return "Zweiter Waermeerzeuger";
        case 2: return "Party";
        case 3: return "Ferien";
        case 4: return "Aus";
        default: return "Unbekannt";
    }
}

void LuxtronikV1Component::parse_modus_heizung_message_(const char* message) {
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_HEATING_MODE)) {
        return;
    }
    
    // values[0] is count, values[1] is the mode value
    if (values.size() >= 2) { 
        float val = std::atof(values[1].c_str()); // Data is at index 1
        std::string mode_text = get_modus_text_(static_cast<int>(val));

        // Only update if values have changed
        bool should_update = false;

        // Check numeric sensor
        if (modus_heizung_numerisch_ != nullptr) {
            if (std::isnan(modus_heizung_numerisch_->state) || modus_heizung_numerisch_->state != val) {
                publish_state_deferred_(modus_heizung_numerisch_, val, "Mode", "Heizung Numerisch");
                should_update = true;
            }
        }

        // Check text sensor
        if (modus_heizung_ != nullptr) {
            if (modus_heizung_->state != mode_text) {
                publish_text_state_deferred_(modus_heizung_, mode_text, "Mode", "Heizung");
                should_update = true;
            }
        }

        // Update select only if value changed
        if (modus_heizung_select_ != nullptr && should_update) {
            modus_heizung_select_->publish_state(mode_text);
            ESP_LOGV(TAG, "Mode Heizung Select updated to: %s", mode_text.c_str());
        }
    }
    
    this->parent_->write_str(std::string(CMD_GET_HOT_WATER_MODE) + "\r\n");
}

void LuxtronikV1Component::parse_modus_brauchwasser_message_(const char* message) {
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_HOT_WATER_MODE)) {
        return;
    }
    
    // values[0] is count, values[1] is the mode value
    if (values.size() >= 2) {
        float val = std::atof(values[1].c_str()); // Data is at index 1
        std::string mode_text = get_modus_text_(static_cast<int>(val));

        // Only update if values have changed
        bool should_update = false;

        // Check numeric sensor
        if (modus_brauchwasser_numerisch_ != nullptr) {
            if (std::isnan(modus_brauchwasser_numerisch_->state) || modus_brauchwasser_numerisch_->state != val) {
                publish_state_deferred_(modus_brauchwasser_numerisch_, val, "Mode", "Brauchwasser Numerisch");
                should_update = true;
            }
        }

        // Check text sensor
        if (modus_brauchwasser_ != nullptr) {
            if (modus_brauchwasser_->state != mode_text) {
                publish_text_state_deferred_(modus_brauchwasser_, mode_text, "Mode", "Brauchwasser");
                should_update = true;
            }
        }

        // Update select only if value changed
        if (modus_brauchwasser_select_ != nullptr && should_update) {
            modus_brauchwasser_select_->publish_state(mode_text);
            ESP_LOGV(TAG, "Mode Brauchwasser Select updated to: %s", mode_text.c_str());
        }
    }
    
    this->parent_->write_str(std::string(CMD_GET_STATUS) + "\r\n");
}

std::string LuxtronikV1Component::get_betriebszustand_text_(int state) {
    switch (state) {
        case 0: return "Heizen";
        case 1: return "Brauchwasser";
        case 3: return "EVU Sperre";
        case 5: return "Bereitschaft";
        default: return "Unbekannt";
    }
}

void LuxtronikV1Component::parse_status_message_(const char* message) {
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_STATUS)) {
        return;
    }
    
    if (values.size() < 2) { // Need at least count and one data value
        ESP_LOGW(TAG, "Status message '%s' has insufficient data after splitting.", msg_str.c_str());
        return;
    }
    
    size_t idx = 1;  // Data starts from index 1, as values[0] is the count

    auto publish_status = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            publish_state_deferred_(sensor, val, "Status", name);
        }
    };

    // Process Anlagentyp
    if (idx < values.size()) publish_status(status_anlagentyp_, values[idx++], "Anlagentyp");

    // special handling because Softwareversion is a string
    if (idx < values.size()) {
        if (status_softwareversion_ != nullptr) {
            publish_text_state_deferred_(status_softwareversion_, values[idx], "Status", "Softwareversion");
            // ESP_LOGV(TAG, "Status Softwareversion: %s", values[idx].c_str()); // Redundant, publish_text_state_deferred_ logs
        }
        idx++;
    }

    // Process Bivalenzstufe
    if (idx < values.size()) publish_status(status_bivalenzstufe_, values[idx++], "Bivalenzstufe");

    // special handling because Betriebszustand has a numeric and a string value
    if (idx < values.size()) {
        float val = std::atof(values[idx].c_str());
        if (status_betriebszustand_numerisch_ != nullptr) {
            publish_state_deferred_(status_betriebszustand_numerisch_, val, "Status", "Betriebszustand Numerisch");
        }
        if (status_betriebszustand_ != nullptr) {
            std::string state_text = get_betriebszustand_text_(static_cast<int>(val));
            publish_text_state_deferred_(status_betriebszustand_, state_text, "Status", "Betriebszustand");
        }
        idx++;
    }

    // Process Letzter Start
    // After previous fields, idx should point to the start of 'Letzter Start' data if available.
    // The timestamp is expected in 6 separate fields: Tag;Monat;Jahr;Stunde;Minute;Sekunde
    if (status_letzter_start_ != nullptr) {
        if (idx + 5 < values.size()) { // Check for all 6 parts of the timestamp
            int tag = std::atoi(values[idx++].c_str());
            int monat = std::atoi(values[idx++].c_str());
            int jahr = std::atoi(values[idx++].c_str());
            int stunde = std::atoi(values[idx++].c_str());
            int minute = std::atoi(values[idx++].c_str());
            int sekunde = std::atoi(values[idx++].c_str());
            
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d %02d:%02d:%02d", 
                     tag, monat, jahr, stunde, minute, sekunde);
            
            publish_timestamp_state_deferred_(status_letzter_start_, buffer, "Status", "Letzter Start");
        } else {
            ESP_LOGW(TAG, "Status message: Incomplete 'Letzter Start' timestamp data. Expected 6 fields, got %zu starting at index %zu. Skipping.", values.size() - idx, idx);
            idx = values.size(); 
        }
    } else {
        // If sensor is null, but data might exist, advance idx accordingly if it's fixed length
        if (idx + 5 < values.size()) {
             idx +=6; // Advance past the 6 fields for timestamp
        } else {
            idx = values.size(); 
        }
    }

    // Request error values after status values are parsed
    this->parent_->write_str(std::string(CMD_GET_ERRORS_BASE) + "\r\n");
}

// Keeping existing splitting logic for parse_error_message_ due to its unique structure
void LuxtronikV1Component::parse_error_message_(const char* message) {
    std::string msg(message);
    std::vector<std::string> values;
    // CMD_GET_ERRORS_BASE is "1500". Error messages can also be 1501, 1502 etc.
    // The first part of the message IS the command number for errors.
    // So we don't use split_message here which expects a fixed prefix.
    // Instead, we find the first semicolon to get the actual command/error index.
    size_t first_semicolon = msg.find(';');
    if (first_semicolon == std::string::npos) return;
    // The part before the first semicolon is the error_index_str (e.g. "1500", "1501")
    std::string error_index_str = msg.substr(0, first_semicolon);
    // ESP_LOGD(TAG, "Error index string: %s", error_index_str.c_str());
    
    // The rest of the message is split by semicolons.
    size_t start = first_semicolon + 1;
    size_t end = 0;

    // Split message into vector for faster processing
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }

    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }

    if (values.size() < 3) return;  // At least count and one value needed

    auto publish_output = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            publish_state_deferred_(sensor, val, "Output", name);
        }
    };

    // Process Fehlerindex
    int error_index_val = std::atoi(error_index_str.c_str()); // Convert the extracted error index string to int
    // idx now refers to the elements in 'values' which are *after* the first semicolon
    size_t idx = 0; 

    switch (error_index_val) { // Use the integer value of the error index
        case 1500: { // This specific case might mean the general error message structure, or error 1500 itself
            // The original code structure implies values[0] is count, values[1] is data, if error_index was from values.
            // Now, values[0] is the count of items *after* "1500;"
            // So, if values[0] is "count", then actual data starts from values[1]
            if (values.empty()) break; // Should have at least count
            idx = 1; // Skip count (values[0]), so data starts at values[1]
            
            // values[idx] (originally values[1]) is expected to be the Fehlercode
            std::string fehlercode_str;
            if (idx < values.size()) {
                fehlercode_str = values[idx];
                if (error0_fehlercode_ != nullptr) {
                    publish_output(error0_fehlercode_, fehlercode_str, "error0_fehlercode");
                }
                // Fehlerbeschreibung uses the same fehlercode_str for lookup
                if (error0_fehlerbeschreibung_ != nullptr) {
                    std::string error_text = get_error_description_(std::atoi(fehlercode_str.c_str()));
                    publish_text_state_deferred_(error0_fehlerbeschreibung_, error_text, "Error", "Fehlerbeschreibung 0");
                }
                idx++; // Advance idx past Fehlercode
            } else {
                ESP_LOGW(TAG, "Error message (1500): Missing Fehlercode data at index %zu. Total values: %zu.", idx, values.size());
                break; 
            }
              
            // After Fehlercode, values[idx] through values[idx+4] are for Zeitpunkt: Tag;Monat;Jahr;Stunde;Minute
            if (error0_zeitpunkt_ != nullptr) {
                if (idx + 4 < values.size()) { // Check for all 5 parts of the timestamp
                    int tag = std::atoi(values[idx++].c_str()); 
                    int monat = std::atoi(values[idx++].c_str());
                    int jahr = std::atoi(values[idx++].c_str());
                    int stunde = std::atoi(values[idx++].c_str());
                    int minute = std::atoi(values[idx++].c_str());

                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d %02d:%02d",
                             tag, monat, jahr, stunde, minute);
                    publish_timestamp_state_deferred_(error0_zeitpunkt_, buffer, "Error", "Zeitpunkt 0");
                } else {
                    ESP_LOGW(TAG, "Error message (1500): Incomplete Zeitpunkt data. Expected 5 fields, got %zu starting at index %zu.", values.size() - idx, idx);
                    idx = values.size(); // Stop processing further fields for this error
                }
            } else {
                // If sensor is null, but data might exist, advance idx accordingly if it's fixed length
                if (idx + 4 < values.size()) {
                    idx += 5;
                } else {
                    idx = values.size();
                }
            }
            break;
        }
        case 1501: {
            if (values.empty()) break;
            idx = 1; // Skip count (values[0]), so data starts at values[1]

            // values[idx] (originally values[1]) is expected to be the Fehlercode
            std::string fehlercode_str;
            if (idx < values.size()) {
                fehlercode_str = values[idx];
                if (error1_fehlercode_ != nullptr) {
                    publish_output(error1_fehlercode_, fehlercode_str, "error1_fehlercode");
                }
                if (error1_fehlerbeschreibung_ != nullptr) {
                    std::string error_text = get_error_description_(std::atoi(fehlercode_str.c_str()));
                    publish_text_state_deferred_(error1_fehlerbeschreibung_, error_text, "Error", "Fehlerbeschreibung 1");
                }
                idx++; // Advance idx past Fehlercode
            } else {
                ESP_LOGW(TAG, "Error message (1501): Missing Fehlercode data at index %zu. Total values: %zu.", idx, values.size());
                break;
            }

            // After Fehlercode, values[idx] through values[idx+4] are for Zeitpunkt: Tag;Monat;Jahr;Stunde;Minute
            if (error1_zeitpunkt_ != nullptr) {
                if (idx + 4 < values.size()) { // Check for all 5 parts of the timestamp
                    int tag = std::atoi(values[idx++].c_str()); 
                    int monat = std::atoi(values[idx++].c_str());
                    int jahr = std::atoi(values[idx++].c_str());
                    int stunde = std::atoi(values[idx++].c_str());
                    int minute = std::atoi(values[idx++].c_str());
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d %02d:%02d",
                             tag, monat, jahr, stunde, minute);
                    publish_timestamp_state_deferred_(error1_zeitpunkt_, buffer, "Error", "Zeitpunkt 1");
                } else {
                    ESP_LOGW(TAG, "Error message (1501): Incomplete Zeitpunkt data. Expected 5 fields, got %zu starting at index %zu.", values.size() - idx, idx);
                    idx = values.size();
                }
            } else {
                // If sensor is null, but data might exist, advance idx to skip these fields
                if (idx + 4 < values.size()) { idx += 5; } else { idx = values.size(); }
            }
            break;
        }
        case 1502: {
            if (values.empty()) break;
            idx = 1; // Skip count (values[0]), so data starts at values[1]

            // values[idx] (originally values[1]) is expected to be the Fehlercode
            std::string fehlercode_str;
            if (idx < values.size()) {
                fehlercode_str = values[idx];
                if (error2_fehlercode_ != nullptr) {
                    publish_output(error2_fehlercode_, fehlercode_str, "error2_fehlercode");
                }
                if (error2_fehlerbeschreibung_ != nullptr) {
                    std::string error_text = get_error_description_(std::atoi(fehlercode_str.c_str()));
                    publish_text_state_deferred_(error2_fehlerbeschreibung_, error_text, "Error", "Fehlerbeschreibung 2");
                }
                idx++; // Advance idx past Fehlercode
            } else {
                ESP_LOGW(TAG, "Error message (1502): Missing Fehlercode data at index %zu. Total values: %zu.", idx, values.size());
                break;
            }

            // After Fehlercode, values[idx] through values[idx+4] are for Zeitpunkt: Tag;Monat;Jahr;Stunde;Minute
            if (error2_zeitpunkt_ != nullptr) {
                if (idx + 4 < values.size()) { // Check for all 5 parts of the timestamp
                    int tag = std::atoi(values[idx++].c_str()); 
                    int monat = std::atoi(values[idx++].c_str());
                    int jahr = std::atoi(values[idx++].c_str());
                    int stunde = std::atoi(values[idx++].c_str());
                    int minute = std::atoi(values[idx++].c_str());
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d %02d:%02d",
                             tag, monat, jahr, stunde, minute);
                    publish_timestamp_state_deferred_(error2_zeitpunkt_, buffer, "Error", "Zeitpunkt 2");
                } else {
                    ESP_LOGW(TAG, "Error message (1502): Incomplete Zeitpunkt data. Expected 5 fields, got %zu starting at index %zu.", values.size() - idx, idx);
                    idx = values.size();
                }
            } else {
                if (idx + 4 < values.size()) { idx += 5; } else { idx = values.size(); }
            }
            break;
        }
        case 1503: {
            if (values.empty()) break;
            idx = 1; // Skip count (values[0]), so data starts at values[1]

            // values[idx] (originally values[1]) is expected to be the Fehlercode
            std::string fehlercode_str;
            if (idx < values.size()) {
                fehlercode_str = values[idx];
                if (error3_fehlercode_ != nullptr) {
                    publish_output(error3_fehlercode_, fehlercode_str, "error3_fehlercode");
                }
                if (error3_fehlerbeschreibung_ != nullptr) {
                    std::string error_text = get_error_description_(std::atoi(fehlercode_str.c_str()));
                    publish_text_state_deferred_(error3_fehlerbeschreibung_, error_text, "Error", "Fehlerbeschreibung 3");
                }
                idx++; // Advance idx past Fehlercode
            } else {
                ESP_LOGW(TAG, "Error message (1503): Missing Fehlercode data at index %zu. Total values: %zu.", idx, values.size());
                break;
            }

            // After Fehlercode, values[idx] through values[idx+4] are for Zeitpunkt: Tag;Monat;Jahr;Stunde;Minute
            if (error3_zeitpunkt_ != nullptr) {
                if (idx + 4 < values.size()) { // Check for all 5 parts of the timestamp
                    int tag = std::atoi(values[idx++].c_str()); 
                    int monat = std::atoi(values[idx++].c_str());
                    int jahr = std::atoi(values[idx++].c_str());
                    int stunde = std::atoi(values[idx++].c_str());
                    int minute = std::atoi(values[idx++].c_str());
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d %02d:%02d",
                             tag, monat, jahr, stunde, minute);
                    publish_timestamp_state_deferred_(error3_zeitpunkt_, buffer, "Error", "Zeitpunkt 3");
                } else {
                    ESP_LOGW(TAG, "Error message (1503): Incomplete Zeitpunkt data. Expected 5 fields, got %zu starting at index %zu.", values.size() - idx, idx);
                    idx = values.size();
                }
            } else {
                if (idx + 4 < values.size()) { idx += 5; } else { idx = values.size(); }
            }
            break;
        }
        case 1504: {
            if (values.empty()) break;
            idx = 1; // Skip count (values[0]), so data starts at values[1]

            // values[idx] (originally values[1]) is expected to be the Fehlercode
            std::string fehlercode_str;
            if (idx < values.size()) {
                fehlercode_str = values[idx];
                if (error4_fehlercode_ != nullptr) {
                    publish_output(error4_fehlercode_, fehlercode_str, "error4_fehlercode");
                }
                if (error4_fehlerbeschreibung_ != nullptr) {
                    std::string error_text = get_error_description_(std::atoi(fehlercode_str.c_str()));
                    publish_text_state_deferred_(error4_fehlerbeschreibung_, error_text, "Error", "Fehlerbeschreibung 4");
                }
                idx++; // Advance idx past Fehlercode
            } else {
                ESP_LOGW(TAG, "Error message (1504): Missing Fehlercode data at index %zu. Total values: %zu.", idx, values.size());
                break;
            }

            // After Fehlercode, values[idx] through values[idx+4] are for Zeitpunkt: Tag;Monat;Jahr;Stunde;Minute
            if (error4_zeitpunkt_ != nullptr) {
                if (idx + 4 < values.size()) { // Check for all 5 parts of the timestamp
                    int tag = std::atoi(values[idx++].c_str()); 
                    int monat = std::atoi(values[idx++].c_str());
                    int jahr = std::atoi(values[idx++].c_str());
                    int stunde = std::atoi(values[idx++].c_str());
                    int minute = std::atoi(values[idx++].c_str());
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d %02d:%02d",
                             tag, monat, jahr, stunde, minute);
                    publish_timestamp_state_deferred_(error4_zeitpunkt_, buffer, "Error", "Zeitpunkt 4");
                } else {
                    ESP_LOGW(TAG, "Error message (1504): Incomplete Zeitpunkt data. Expected 5 fields, got %zu starting at index %zu.", values.size() - idx, idx);
                    idx = values.size();
                }
            } else {
                if (idx + 4 < values.size()) { idx += 5; } else { idx = values.size(); }
            }
            break;
        }
        default: {
            ESP_LOGW(TAG, "Unknown error index value: %d in message: %s", error_index_val, msg.c_str());
            // Removed extra "tag, monat, jahr, stunde, minute" from the log message as they are not defined in this scope.
            break;
        }
    }
    // Request operating hours after error values are parsed
    this->parent_->write_str(std::string(CMD_GET_OPERATING_HOURS) + "\r\n");
}

void LuxtronikV1Component::parse_operatinghours_message_(const char* message) {
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_OPERATING_HOURS)) {
        return;
    }

    if (values.size() < 2) { // Need at least count and one data value
        ESP_LOGW(TAG, "Operating hours message '%s' has insufficient data after splitting.", msg_str.c_str());
        return;
    }

    size_t idx = 1;  // Data starts from index 1, as values[0] is the count
    
    auto publish_hours = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            // Convert seconds to hours
            float hours = val / 3600.0f;
            publish_state_deferred_(sensor, hours, "Output", name);
        }
    };

    auto publish_impulses = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            // Impulses are published 1:1
            publish_state_deferred_(sensor, val, "Output", name);
        }
    };

    // Process Betriebsstunden
    if (idx < values.size()) publish_hours(betriebsstunden_verdichter_1_, values[idx++], "Betriebsstunden Verdichter 1");
    if (idx < values.size()) publish_impulses(impulse_verdichter_1_, values[idx++], "Impulse Verdichter 1");
    if (idx < values.size()) publish_hours(durchschnittliche_einschaltdauer_verdichter_1_, values[idx++], "Durchschnittliche Einschaltdauer Verdichter 1");
    if (idx < values.size()) publish_hours(betriebsstunden_verdichter_2_, values[idx++], "Betriebsstunden Verdichter 2");
    if (idx < values.size()) publish_impulses(impulse_verdichter_2_, values[idx++], "Impulse Verdichter 2");
    if (idx < values.size()) publish_hours(durchschnittliche_einschaltdauer_verdichter_2_, values[idx++], "Durchschnittliche Einschaltdauer Verdichter 2");
    if (idx < values.size()) publish_hours(betriebsstunden_zweiter_waermeerzeuger_1_, values[idx++], "Betriebsstunden Zweiter Waermeerzeuger 1");
    if (idx < values.size()) publish_hours(betriebsstunden_zweiter_waermeerzeuger_2_, values[idx++], "Betriebsstunden Zweiter Waermeerzeuger 2");
    if (idx < values.size()) publish_hours(betriebsstunden_waermepumpe_, values[idx++], "Betriebsstunden Waermepumpe");

    // Request heating curve after operating hour values are parsed
    this->parent_->write_str(std::string(CMD_GET_HEATING_CURVE) + "\r\n");
}

void LuxtronikV1Component::parse_heatingcurve_message_(const char* message) { 
    // Received: 3400;9;20;310;200;0;350;340;200;0;350
    std::string msg_str(message);
    std::vector<std::string> values;
    if (!split_message_(msg_str, values, CMD_GET_HEATING_CURVE)) {
        return;
    }

    if (values.size() < 2) { // Need at least count and one data value
        ESP_LOGW(TAG, "Heating curve message '%s' has insufficient data after splitting.", msg_str.c_str());
        return;
    }

    size_t idx = 1;  // Data starts from index 1, as values[0] is the count

    auto publish_temp = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float temp = get_float_temp_(value);
            publish_state_deferred_(sensor, temp, "Temperatur", name);
        }
    };

    // Process Heizkurve
    if (idx < values.size()) publish_temp(heizkurve_temperaturdelta_, values[idx++], "Heizkurve Temperaturdelta");
    if (idx < values.size()) publish_temp(heizkurve_endpunkt_, values[idx++], "Heizkurve Endpunkt");
    if (idx < values.size()) publish_temp(heizkurve_parallelverschiebung_, values[idx++], "Heizkurve Parallelverschiebung");
    if (idx < values.size()) publish_temp(heizkurve_absenkung_, values[idx++], "Heizkurve Absenkung");
    if (idx < values.size()) publish_temp(heizkurve_festwert_ruecklauf_, values[idx++], "Heizkurve Festwert Rücklauf");
    if (idx < values.size()) publish_temp(mischkreis1_heizkurvenendpunkt_, values[idx++], "Mischkreis1 Heizkurvenendpunkt");
    if (idx < values.size()) publish_temp(mischkreis1_parallelverschiebung_, values[idx++], "Mischkreis1 Parallelverschiebung");
    if (idx < values.size()) publish_temp(mischkreis1_absenkung_, values[idx++], "Mischkreis1 Absenkung");
    if (idx < values.size()) publish_temp(mischkreis1_festwert_vorlauf_, values[idx++], "Mischkreis1 Festwert Vorlauf");
}  

// Keeping existing splitting logic for reset_programming_mode_ due to its unique structure
void LuxtronikV1Component::reset_programming_mode_(const char* message) {
    // Example message format:
    // Received: 779;3506;1
    // to reset programming mode, send "<CMD_PROGRAMMING_ERROR_PREFIX>;<mode_val>;0" to the device followed by "<CMD_SAVE_PROGRAMMING>\r\n"
    // e.g. "779;3506;0\r\n" then "999\r\n"

    ESP_LOGD(TAG, "Programming-Errormessage received: %s", message);
    ESP_LOGD(TAG, "Reset programming mode...");

    std::string msg(message);
    std::vector<std::string> values;
    size_t prefix_len = strlen(CMD_PROGRAMMING_ERROR_PREFIX);
    if (msg.rfind(CMD_PROGRAMMING_ERROR_PREFIX, 0) != 0 || msg.length() <= prefix_len || msg[prefix_len] != ';') {
        ESP_LOGW(TAG, "Invalid programming error message format: %s", message);
        return;
    }
    size_t start = prefix_len + 1;  // Skip "779;"
    size_t end = 0;
  
    // Split message into vector for faster processing
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }

    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }

    if (this->parent_ != nullptr && values.size() >= 1) {
        // Get the mode number (3406 or 3506)
        std::string mode = values[0];
        
        // Create reset command with corresponding mode
        char command[32];
        snprintf(command, sizeof(command), "%s;0\r\n", mode.c_str());
        
        // Send reset command followed by confirmation
        this->parent_->write_str(command);
        // Allow time for the Luxtronik to process the mode reset command before sending confirmation.
        // This delay may need to be adjusted based on the device's response time.
        delay(500);
        // Send confirmation command
        this->parent_->write_str(std::string(CMD_SAVE_PROGRAMMING) + "\r\n");
    }
}

// Move control implementations to cpp file
void ModusBrauchwasserSelect::control(const std::string &value) {
    // Example message format:
    // Send "<CMD_PROGRAM_HOT_WATER_MODE>;1" to set programming mode for water heating
    // Send "<CMD_PROGRAM_HOT_WATER_MODE>;1;<MODE>" to program mode
    // Send "<CMD_SAVE_PROGRAMMING>" to save the new mode

    if (parent_ == nullptr) return;
    
    int mode_val = 0;  // Default to Automatik
    if (value == "Zweiter Waermeerzeuger") mode_val = 1;
    else if (value == "Party") mode_val = 2;
    else if (value == "Ferien") mode_val = 3;
    else if (value == "Aus") mode_val = 4;
    
    char command_buffer[32];
    // Set programming mode for water heating
    snprintf(command_buffer, sizeof(command_buffer), "%s;1\r\n", CMD_PROGRAM_HOT_WATER_MODE);
    parent_->write_str(command_buffer);
    // Brief delay to allow Luxtronik to process the previous command.
    delay(100);

    // Send new mode to heatpump
    snprintf(command_buffer, sizeof(command_buffer), "%s;1;%d\r\n", CMD_PROGRAM_HOT_WATER_MODE, mode_val);
    parent_->write_str(command_buffer);
    
    // Brief delay to allow Luxtronik to process the previous command.
    delay(100);
    
    // Send save command
    parent_->write_str(std::string(CMD_SAVE_PROGRAMMING) + "\r\n");
    
    ESP_LOGD("luxtronik_v1", "Changed Brauchwasser mode to: %s (Mode: %d)", value.c_str(), mode_val);
  }
  
  void ModusHeizungSelect::control(const std::string &value) {
    // Example message format:
    // Send "<CMD_PROGRAM_HEATING_MODE>;1" to set programming mode for heating
    // Send "<CMD_PROGRAM_HEATING_MODE>;1;<MODE>" to program mode
    // Send "<CMD_SAVE_PROGRAMMING>" to save the new mode
    
    if (parent_ == nullptr) return;
    
    int mode_val = 0;  // Default to Automatik
    if (value == "Zweiter Waermeerzeuger") mode_val = 1;
    else if (value == "Party") mode_val = 2;
    else if (value == "Ferien") mode_val = 3;
    else if (value == "Aus") mode_val = 4;
    
    char command_buffer[32];
    // Set programming mode for heatingmode
    snprintf(command_buffer, sizeof(command_buffer), "%s;1\r\n", CMD_PROGRAM_HEATING_MODE);
    parent_->write_str(command_buffer);
    // Brief delay to allow Luxtronik to process the previous command.
    delay(100);
 
    // Send new mode to heatpump
    snprintf(command_buffer, sizeof(command_buffer), "%s;1;%d\r\n", CMD_PROGRAM_HEATING_MODE, mode_val);
    parent_->write_str(command_buffer);
    
    // Brief delay to allow Luxtronik to process the previous command.
    delay(100);
    
    // Send save command
    parent_->write_str(std::string(CMD_SAVE_PROGRAMMING) + "\r\n");
    
    ESP_LOGD("luxtronik_v1", "Changed Heizung mode to: %s (Mode: %d)", value.c_str(), mode_val);
  }
  
// Add after ModusHeizungSelect implementation:

void WarmwasserSolltemperaturNumber::control(float value) {
  if (parent_ == nullptr) return;

  // Convert float to integer (multiplied by 10 as protocol expects)
  int temp_val = static_cast<int>(value * 10);
  
  char command_buffer[32];
  // Set programming mode for water heating temperature
  snprintf(command_buffer, sizeof(command_buffer), "%s;1\r\n", CMD_PROGRAM_HOT_WATER_TEMP);
  parent_->write_str(command_buffer);
  // Brief delay to allow Luxtronik to process the previous command.
  delay(100);

  // Send new temperature to heat pump
  snprintf(command_buffer, sizeof(command_buffer), "%s;1;%d\r\n", CMD_PROGRAM_HOT_WATER_TEMP, temp_val);
  parent_->write_str(command_buffer);
  
  // Brief delay to allow Luxtronik to process the previous command.
  delay(100);
  
  // Send save command
  parent_->write_str(std::string(CMD_SAVE_PROGRAMMING) + "\r\n");
  
  ESP_LOGD("luxtronik_v1", "Changed Warmwasser Solltemperatur to: %.1f°C", value);
}

  
void LuxtronikV1Component::dump_config() {
    ESP_LOGCONFIG(TAG, "Luxtronik V1 Component:");
    ESP_LOGCONFIG(TAG, "  UART Parent: %s", this->parent_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Vorlauf: %s", this->temperatur_vorlauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Rücklauf: %s", this->temperatur_ruecklauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Rücklauf Soll: %s", this->temperatur_ruecklauf_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Heissgas: %s", this->temperatur_heissgas_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Aussen: %s", this->temperatur_aussen_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Brauchwasser: %s", this->temperatur_brauchwasser_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Brauchwasser Soll: %s", this->temperatur_brauchwasser_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Wärmequelle Eingang: %s", this->temperatur_waermequelle_eingang_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Kältekreis: %s", this->temperatur_kaeltekreis_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Mischkreis1 Vorlauf: %s", this->temperatur_mischkreis1_vorlauf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Mischkreis1 Vorlauf Soll: %s", this->temperatur_mischkreis1_vorlauf_soll_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Temperatur Raumstation: %s", this->temperatur_raumstation_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Abtau Soledruck Durchfluss: %s", this->eingang_abtau_soledruck_durchfluss_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Sperrzeit EVU: %s", this->eingang_sperrzeit_evu_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Hochdruckpressostat: %s", this->eingang_hochdruckpressostat_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Motorschutz: %s", this->eingang_motorschutz_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Niederdruckpressostat: %s", this->eingang_niederdruckpressostat_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Eingang Fremdstromanode: %s", this->eingang_fremdstromanode_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Abtauventil: %s", this->ausgang_abtauventil_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Brauchwasserpumpe: %s", this->ausgang_brauchwasserpumpe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Fussbodenheizungspumpe: %s", this->ausgang_fussbodenheizungspumpe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Heizungspumpe: %s", this->ausgang_heizungspumpe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Mischer 1 Auf: %s", this->ausgang_mischer_1_auf_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Mischer 1 Zu: %s", this->ausgang_mischer_1_zu_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Ventilator Waermepumpe: %s", this->ausgang_ventilator_waermepumpe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Ventilator Brunnen: %s", this->ausgang_ventilator_brunnen_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Verdichter 1: %s", this->ausgang_verdichter_1_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Verdichter 2: %s", this->ausgang_verdichter_2_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Zirkulationspumpe: %s", this->ausgang_zirkulationspumpe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Zweiter Wärmeerzeuger: %s", this->ausgang_zweiter_waermeerzeuger_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Ausgang Zweiter Wärmeerzeuger Störung: %s", this->ausgang_zweiter_waermeerzeuger_stoerung_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Modus Heizung Numerisch: %s", this->modus_heizung_numerisch_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Modus Heizung: %s", this->modus_heizung_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Modus Brauchwasser Numerisch: %s", this->modus_brauchwasser_numerisch_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Modus Brauchwasser: %s", this->modus_brauchwasser_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Anlagentyp: %s", this->status_anlagentyp_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Softwareversion: %s", this->status_softwareversion_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Bivalenzstufe: %s", this->status_bivalenzstufe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Betriebszustand Numerisch: %s", this->status_betriebszustand_numerisch_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Betriebszustand: %s", this->status_betriebszustand_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Letzter Start: %s", this->status_letzter_start_ ? "Set" : "Not Set");
}

std::string LuxtronikV1Component::get_error_description_(int error_code) {
    switch (error_code) {
        case 701: return "Niederdruckstörung - Niederdruckpressostat oder -sensor hat mehrfach ausgelöst.";
        case 702: return "Niederdrucksperre (Reset auto.) - Niederdruck hat angesprochen, automatischer Neustart.";
        case 703: return "Frostschutz - Vorlauftemperatur < 5°C erkannt.";
        case 704: return "Heißgasstörung - Max. Temperatur im Heißgaskreis überschritten.";
        case 705: return "Motorschutz VEN - Motorschutz des Ventilators hat ausgelöst.";
        case 706: return "Motorschutz BSUP - Motorschutz der Sole-/Brunnenwasserpumpe oder Verdichter.";
        case 707: return "Kodierungsfehler WP - Kodierungswiderstand oder Verbindung fehlerhaft.";
        case 708: return "Fühler Rücklauf - Bruch/Kurzschluss des Rücklauffühlers.";
        case 709: return "Fühler Vorlauf - Bruch/Kurzschluss des Vorlauffühlers.";
        case 710: return "Fühler Heißgas - Bruch/Kurzschluss des Heißgasfühlers.";
        case 711: return "Fühler Außentemperatur - Bruch/Kurzschluss des Außentemperaturfühlers.";
        case 712: return "Fühler Trinkwasser - Bruch/Kurzschluss des Trinkwasserfühlers.";
        case 713: return "Fühler WQ-Eintritt - Bruch/Kurzschluss des Wärmequellenfühlers (Eintritt).";
        case 714: return "Heißgas WW - Temperaturgrenze Trinkwasser überschritten.";
        case 715: return "Hochdruck-Abschaltung (Reset) - Hochdruckpressostat hat angesprochen.";
        case 716: return "Hochdruckstörung - Hochdruckpressostat mehrfach angesprochen.";
        case 717: return "Durchfluss-WQ - Durchflussschalter hat angesprochen.";
        case 718: return "Max. Außentemp. (Reset) - Außentemperatur überschritten.";
        case 719: return "Min. Außentemp. (Reset) - Außentemperatur unterschritten.";
        case 720: return "WQ-Temperatur (Reset) - Verdampferaustrittstemp. mehrfach unter Sicherheitswert.";
        case 721: return "Niederdruckabsenkung (Reset) - Niederdruckpressostat oder -sensor hat angesprochen.";
        case 722: return "Tempdiff Heizwasser - Temperaturspreizung im Heizbetrieb ist negativ.";
        case 723: return "Tempdiff Warmw. - Temperaturspreizung im Trinkwasserbetrieb ist negativ.";
        case 724: return "Tempdiff Abtauen - Temperaturspreizung im Heizkreis ist während des Abtauens > 15 K.";
        case 725: return "Anlagefehler WW - Trinkwasserbetrieb gestört, gewünschte Speichertemperatur ist weit unterschritten.";
        case 726: return "Fühler Mischkreis 1 - Bruch oder Kurzschluss des Mischkreisfühlers.";
        case 727: return "Soledruck - Soledruckpressostat hat angesprochen.";
        case 728: return "Fühler WQ-Aus - Bruch oder Kurzschluss des Wärmequellenfühlers (Austritt).";
        case 729: return "Drehfeldfehler - Verdichter nach dem Einschalten ohne Leistung.";
        case 730: return "Leistung Ausheizen - Ausheizprogramm konnte eine VL-Temperaturstufe nicht erreichen.";
        case 731: return "Zeitüberschreitung TDI - Thermische Desinfektion konnte nicht durchgeführt werden.";
        case 732: return "Störung Kühlung - Heizwassertemperatur von 16°C mehrfach unterschritten.";
        case 733: return "Störung Anode - Störmeldeeingang der Fremdstromanode hat angesprochen.";
        case 734: return "Störung Anode - Fehler liegt seit mehr als zwei Wochen an, Trinkwasserbereitung gesperrt.";
        case 735: return "Fühler Ext. En - Bruch oder Kurzschluss des Fühlers 'Externe Energiequelle' (TEE).";
        case 736: return "Fühler Solarkollektor - Bruch oder Kurzschluss des Solarkollektorfühlers.";
        case 737: return "Fühler Solarspeicher - Bruch oder Kurzschluss des Solarspeicherfühlers.";
        case 738: return "Fühler Mischkreis 2 - Bruch oder Kurzschluss des Mischkreisfühlers 2.";
        case 739: return "Fühler Mischkreis 3 - Bruch oder Kurzschluss des Mischkreisfühlers 3.";
        case 750: return "Fühler Rücklauf extern - Bruch oder Kurzschluss des externen Rücklauffühlers.";
        case 751: return "Phasenüberwachungsfehler - Phasenfolgerelais hat angesprochen.";
        case 752: return "Phasenüberwachungs-/Durchflussfehler - Phasenfolgerelais oder Durchflussschalter hat angesprochen.";
        case 755: return "Verbindung zu Slave verloren - Ein Slave hat für mehr als 5 Minuten nicht geantwortet.";
        case 756: return "Verbindung zu Master verloren - Master hat für mehr als 5 Minuten nicht geantwortet.";
        case 757: return "ND-Störung bei W/W-Gerät - Niederdruckpressostat hat mehrfach oder länger als 20 Sekunden angesprochen.";
        case 758: return "Störung Abtauung - Abtauung wurde 5-mal in Folge zu niedriger Vorlauftemperatur beendet.";
        case 759: return "Meldung TDI - Thermische Desinfektion konnte nicht korrekt durchgeführt werden.";
        case 760: return "Störung Abtauung - Abtauung wurde 5-mal in Folge über Maximalzeit beendet.";
        case 761: return "LIN-Verbindung unterbrochen - LIN-Timeout.";
        case 762: return "Fühler Ansaug Verdichter - Fühlerfehler Tü (Ansaug Verdichter).";
        case 763: return "Fühler Ansaug-Verdampfer - Fühlerfehler Tü1 (Ansaug Verdampfer).";
        case 764: return "Fühler Verdichterheizung - Fühlerfehler Verdichterheizung.";
        case 765: return "Überhitzung - Überhitzung länger als 5 Minuten unter 2K.";
        case 766: return "Einsatzgrenzen-VD - Betrieb 5 Minuten außerhalb des Einsatzbereichs des Verdichters.";
        case 767: return "STB E-Stab - STB des Heizstabs wurde aktiviert.";
        case 768: return "Durchflussüberwachung - 5-mal zu geringer Durchfluss vor Abtauung.";
        case 769: return "Pumpenansteuerung - Kein gültiges Durchflusssignal von der Umwälzpumpe.";
        case 770: return "Niedrige Überhitzung - Überhitzung über längere Zeit unter Grenzwert.";
        case 771: return "Hohe Überhitzung - Überhitzung über längere Zeit über Grenzwert.";
        case 776: return "Einsatzgrenzen-VD - Verdichter läuft außerhalb Einsatzgrenzen.";
        case 777: return "Expansionsventil - Expansionsventil defekt.";
        case 778: return "Fühler Niederdruck - Niederdruckfühler defekt.";
        case 779: return "Fühler Hochdruck - Hochdruckfühler defekt.";
        case 780: return "Fühler EVI - EVI-Fühler defekt.";
        case 799: return "ModBus ASB - Keine ModBus-Kommunikation mit ASB-Platine.";
        default: return "Unbekannter Fehler";
    }
}

}  // namespace luxtronik_v1_component
}  // namespace esphome