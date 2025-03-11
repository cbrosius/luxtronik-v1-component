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

    // Request initial values immediately after setup
    this->parent_->write_str("1100\r\n");
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

// Helper function for deferred publishing
void LuxtronikV1Component::publish_state_deferred_(sensor::Sensor* sensor, float value, const char* type, const char* name) {
    this->defer([this, sensor, value, type, name]() {
        sensor->publish_state(value);
        ESP_LOGV(TAG, "%s %s: %.1f", type, name, value);
    });
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
            parse_temperatur_message_(msg.c_str());
        });
    } else if (prefix == "1200") {
        this->defer([this, msg]() {
            parse_input_message_(msg.c_str());
        });
    } else if (prefix == "1300") {
        this->defer([this, msg]() {
            parse_output_message_(msg.c_str());
        });
    } else if (prefix == "3405") {
        this->defer([this, msg]() {
            parse_modus_heizung_message_(msg.c_str());
        });
    } else if (prefix == "3505") {
        this->defer([this, msg]() {
            parse_modus_warmwasser_message_(msg.c_str());
        });
    } else if (prefix == "1700") {
        this->defer([this, msg]() {
            parse_status_message_(msg.c_str());
        });
    }
}

void LuxtronikV1Component::parse_temperatur_message_(const char* message) {
    std::string msg(message);
    std::vector<std::string> values;
    size_t start = 5;  // Skip "1100;"
    size_t end = 0;
    
    // Split message into vector for faster processing
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    
    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
    auto publish_temp = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float temp = get_float_temp_(value);
            publish_state_deferred_(sensor, temp, "Temperatur", name);
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
    
    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
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
    
    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
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
    this->parent_->write_str("3405\r\n");

}

void LuxtronikV1Component::parse_modus_heizung_message_(const char* message) {
    std::string msg(message);
    std::vector<std::string> values;
    values.reserve(3);  // Pre-allocate for typical message size
    size_t start = 5;  // Skip "3405;"
    size_t end = 0;
    
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    // Add final value if exists
    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }
    
    if (values.size() >= 2) {  // At least count and mode value
        if (modus_heizung_ != nullptr) {
            float val = std::atof(values[1].c_str());
            publish_state_deferred_(modus_heizung_, val, "Mode", "Heizung");
        }
    }
    
    // Request hot water mode after heating mode
    this->parent_->write_str("3505\r\n");
}

void LuxtronikV1Component::parse_modus_warmwasser_message_(const char* message) {
    std::string msg(message);
    std::vector<std::string> values;
    values.reserve(3);  // Pre-allocate for typical message size
    size_t start = 5;  // Skip "3505;"
    size_t end = 0;
    
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    // Add final value if exists
    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }
    
    if (values.size() >= 2) {  // At least count and mode value
        if (modus_warmwasser_ != nullptr) {
            float val = std::atof(values[1].c_str());
            publish_state_deferred_(modus_warmwasser_, val, "Mode", "Warmwasser");
        }
    }
    
    // Request status values after warmwater mode
    this->parent_->write_str("1700\r\n");
}

void LuxtronikV1Component::parse_status_message_(const char* message) {
    std::string msg(message);
    std::vector<std::string> values;
    values.reserve(13);  // Pre-allocate for all status values
    size_t start = 5;  // Skip "1700;"
    size_t end = 0;
    
    while ((end = msg.find(';', start)) != std::string::npos) {
        values.push_back(msg.substr(start, end - start));
        start = end + 1;
    }
    if (start < msg.length()) {
        values.push_back(msg.substr(start));
    }
    
    if (values.size() < 2) return;  // At least count and one value needed
    
    size_t idx = 1;  // Skip count
    auto publish_status = [this](sensor::Sensor* sensor, const std::string& value, const char* name) {
        if (sensor != nullptr) {
            float val = std::atof(value.c_str());
            publish_state_deferred_(sensor, val, "Status", name);
        }
    };

    // Process all status sensors
    if (idx < values.size()) publish_status(status_anlagentyp_, values[idx++], "Anlagentyp");

    // special handling because Softwareversion is a string
    if (idx < values.size()) {
        if (status_softwareversion_ != nullptr) {
            this->defer([this, value = values[idx]]() {
                status_softwareversion_->publish_state(value);
                ESP_LOGV(TAG, "Status Softwareversion: %s", value.c_str());
            });
        }
        idx++;
    }

    if (idx < values.size()) publish_status(status_bivalenzstufe_, values[idx++], "Bivalenzstufe");

    // special handling because Betriebszustand has a numeric and a string value
    if (idx < values.size()) {
        float val = std::atof(values[idx].c_str());
        if (status_betriebszustand_numerisch_ != nullptr) {
            publish_state_deferred_(status_betriebszustand_numerisch_, val, "Status", "Betriebszustand Numerisch");
            if (status_betriebszustand_ != nullptr) {
                switch (status_betriebszustand_numerisch_) {
                    case 0: std::string state_text = "Heizen";
                    case 1: std::string state_text = "Warmwasser";
                    case 3: std::string state_text = "EVU Sperre";
                    case 5: std::string state_text = "Bereitschaft";
                    default: return "Unbekannt";
                }
                this->defer([this, state_text]() {
                    status_betriebszustand_->publish_state(state_text);
                    ESP_LOGV(TAG, "Status Betriebszustand: %s", state_text.c_str());
                });
            }
        }
        idx++;
    }
    if (idx < values.size()) publish_status(status_startdatum_tag_, values[idx++], "Startdatum Tag");
    if (idx < values.size()) publish_status(status_startdatum_monat_, values[idx++], "Startdatum Monat");
    if (idx < values.size()) publish_status(status_startdatum_jahr_, values[idx++], "Startdatum Jahr");
    if (idx < values.size()) publish_status(status_startuhrzeit_std_, values[idx++], "Startuhrzeit Std");
    if (idx < values.size()) publish_status(status_startuhrzeit_min_, values[idx++], "Startuhrzeit Min");
    if (idx < values.size()) publish_status(status_startuhrzeit_sek_, values[idx++], "Startuhrzeit Sek");
    if (idx < values.size()) publish_status(status_compact_, values[idx++], "Compact");
    if (idx < values.size()) publish_status(status_comfort_, values[idx++], "Comfort");
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
    ESP_LOGCONFIG(TAG, "  Sensor Modus Heizung: %s", this->modus_heizung_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Modus Warmwasser: %s", this->modus_warmwasser_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Anlagentyp: %s", this->status_anlagentyp_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Softwareversion: %s", this->status_softwareversion_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Bivalenzstufe: %s", this->status_bivalenzstufe_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Betriebszustand Numerisch: %s", this->status_betriebszustand_numerisch_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Betriebszustand: %s", this->status_betriebszustand_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Startdatum Tag: %s", this->status_startdatum_tag_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Startdatum Monat: %s", this->status_startdatum_monat_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Startdatum Jahr: %s", this->status_startdatum_jahr_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Startuhrzeit Std: %s", this->status_startuhrzeit_std_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Startuhrzeit Min: %s", this->status_startuhrzeit_min_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Startuhrzeit Sek: %s", this->status_startuhrzeit_sek_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Compact: %s", this->status_compact_ ? "Set" : "Not Set");
    ESP_LOGCONFIG(TAG, "  Sensor Status Comfort: %s", this->status_comfort_ ? "Set" : "Not Set");
}

}  // namespace luxtronik_v1_component
}  // namespace esphome