#include "esphome/core/log.h"
#include "luxtronik_v1_sensor.h"

namespace esphome {

namespace luxtronik_v1_sensor {

static const char *TAG = "luxtronik_v1_sensor.sensor";

LuxtronikV1Sensor::LuxtronikV1Sensor() 
    : PollingComponent(60000), uart_(nullptr), temp_VL_ptr(nullptr),
      temp_RL_ptr(nullptr), temp_RL_Soll_ptr(nullptr), temp_Heissgas_ptr(nullptr),
      temp_Aussen_ptr(nullptr), temp_BW_ptr(nullptr),
      temp_BW_Soll_ptr(nullptr), temp_WQ_Ein_ptr(nullptr),
      temp_Kaeltekreis_ptr(nullptr), temp_MK1_Vorl_ptr(nullptr),
      temp_MK1VL_Soll_ptr(nullptr), temp_Raumstat_ptr(nullptr),
      ein_Abtau_Soledruck_Durchfluss_ptr(nullptr), ein_Sperrzeit_EVU_ptr(nullptr),
      ein_Hochdruckpressostat_ptr(nullptr), ein_Motorschutz_ptr(nullptr),
      ein_Niederdruckpressostat_ptr(nullptr), ein_Fremdstromanode_ptr(nullptr),
      aus_ATV_ptr(nullptr), aus_BWP_ptr(nullptr),
      aus_FBHP_ptr(nullptr), aus_HZP_ptr(nullptr),
      aus_Mischer_1_Auf_ptr(nullptr), aus_Mischer_1_Zu_ptr(nullptr),
      aus_VentWP_ptr(nullptr), aus_VentBrunnen_ptr(nullptr),
      aus_Verdichter_1_ptr(nullptr), aus_Verdichter_2_ptr(nullptr),
      aus_ZPumpe_ptr(nullptr), aus_ZWE_ptr(nullptr),
      aus_ZWE_Stoerung_ptr(nullptr), status_Anlagentyp_ptr(nullptr),
      status_Softwareversion_ptr(nullptr), status_Bivalenzstufe_ptr(nullptr),
      status_Betriebszustand_ptr(nullptr), modus_Heizung_ptr(nullptr),
      modus_Warmwasser_ptr(nullptr) {}

void LuxtronikV1Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Luxtronik V1 Sensor...");
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Uart is nullptr");
  } else {
    ESP_LOGI(TAG, "Uart is set.");
  }
  // Clear Read Buffer
  for (size_t i = 0; i < READ_BUFFER_LENGTH; i++) {
    read_buffer_[i] = 0;
  }
  // Send initial command
  send_cmd_("1100");
}

void LuxtronikV1Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Luxtronik V1 Sensor:");
  LOG_SENSOR("  ", "Temperature VL", this->temp_VL_ptr);
  LOG_SENSOR("  ", "Temperature RL", this->temp_RL_ptr);
  LOG_SENSOR("  ", "Temperature RL Soll", this->temp_RL_Soll_ptr);
  LOG_SENSOR("  ", "Temperature Heissgas", this->temp_Heissgas_ptr);
  LOG_SENSOR("  ", "Temperature Aussen", this->temp_Aussen_ptr);
  LOG_SENSOR("  ", "Temperature BW", this->temp_BW_ptr);
  LOG_SENSOR("  ", "Temperature BW Soll", this->temp_BW_Soll_ptr);
  LOG_SENSOR("  ", "Temperature WQ Ein", this->temp_WQ_Ein_ptr);
  LOG_SENSOR("  ", "Temperature Kaeltekreis", this->temp_Kaeltekreis_ptr);
  LOG_SENSOR("  ", "Temperature MK1 Vorlauf", this->temp_MK1_Vorl_ptr);
  LOG_SENSOR("  ", "Temperature MK1 Vorlauf Soll", this->temp_MK1VL_Soll_ptr);
  LOG_SENSOR("  ", "Temperature Raumstat", this->temp_Raumstat_ptr);
  LOG_SENSOR("  ", "Eingang Abtau Soledruck Durchfluss", this->ein_Abtau_Soledruck_Durchfluss_ptr);
  LOG_SENSOR("  ", "Eingang Sperrzeit EVU", this->ein_Sperrzeit_EVU_ptr);
  LOG_SENSOR("  ", "Eingang Hochdruckpressostat", this->ein_Hochdruckpressostat_ptr);
  LOG_SENSOR("  ", "Eingang Motorschutz", this->ein_Motorschutz_ptr);
  LOG_SENSOR("  ", "Eingang Niederdruckpressostat", this->ein_Niederdruckpressostat_ptr);
  LOG_SENSOR("  ", "Eingang Fremdstromanode", this->ein_Fremdstromanode_ptr);
  LOG_SENSOR("  ", "Ausgang ATV", this->aus_ATV_ptr);
  LOG_SENSOR("  ", "Ausgang BWP", this->aus_BWP_ptr);
  LOG_SENSOR("  ", "Ausgang FBHP", this->aus_FBHP_ptr);
  LOG_SENSOR("  ", "Ausgang HZP", this->aus_HZP_ptr);
  LOG_SENSOR("  ", "Ausgang Mischer 1 Auf", this->aus_Mischer_1_Auf_ptr);
  LOG_SENSOR("  ", "Ausgang Mischer 1 Zu", this->aus_Mischer_1_Zu_ptr);
  LOG_SENSOR("  ", "Ausgang Vent WP", this->aus_VentWP_ptr);
  LOG_SENSOR("  ", "Ausgang Vent Brunnen", this->aus_VentBrunnen_ptr);
  LOG_SENSOR("  ", "Ausgang Verdichter 1", this->aus_Verdichter_1_ptr);
  LOG_SENSOR("  ", "Ausgang Verdichter 2", this->aus_Verdichter_2_ptr);
  LOG_SENSOR("  ", "Ausgang ZPumpe", this->aus_ZPumpe_ptr);
  LOG_SENSOR("  ", "Ausgang ZWE", this->aus_ZWE_ptr);
  LOG_SENSOR("  ", "Ausgang ZWE Stoerung", this->aus_ZWE_Stoerung_ptr);
  LOG_SENSOR("  ", "Status Anlagentyp", this->status_Anlagentyp_ptr);
  LOG_SENSOR("  ", "Status Softwareversion", this->status_Softwareversion_ptr);
  LOG_SENSOR("  ", "Status Bivalenzstufe", this->status_Bivalenzstufe_ptr);
  LOG_SENSOR("  ", "Status Betriebszustand", this->status_Betriebszustand_ptr);
  LOG_SENSOR("  ", "Modus Heizung", this->modus_Heizung_ptr);
  LOG_SENSOR("  ", "Modus Warmwasser", this->modus_Warmwasser_ptr);
}

void LuxtronikV1Sensor::loop() {
  if (!this->uart_) {
    ESP_LOGE(TAG, "UART not initialized!");
    return;
  }

  while (available()) {
    uint8_t byte;
    if (!this->read_byte(&byte)) {
      ESP_LOGW(TAG, "Failed to read byte from UART");
      continue;
    }

    ESP_LOGV(TAG, "Received byte: 0x%02X ('%c')", byte, (byte >= 32 && byte < 127) ? byte : '.');

    if (byte == ASCII_CR)
      continue;

    if (this->read_pos_ >= READ_BUFFER_LENGTH - 1) {
      ESP_LOGE(TAG, "Read buffer overflow! Resetting.");
      this->read_pos_ = 0;
      memset(this->read_buffer_, 0, READ_BUFFER_LENGTH);
      continue;
    }

    this->read_buffer_[this->read_pos_] = byte;
    
    // Check if current byte is LF (not checking buffer position)
    if (byte == ASCII_LF) {
      this->read_buffer_[this->read_pos_] = 0;  // Null terminate
      ESP_LOGI(TAG, "Received line: %s", this->read_buffer_);
      this->parse_cmd_(this->read_buffer_);
      memset(this->read_buffer_, 0, READ_BUFFER_LENGTH);
      this->read_pos_ = 0;
    } else {
      this->read_pos_++;
    }
  }
}

void LuxtronikV1Sensor::update() {
  // Ask for Temperatures
  send_cmd_("1100");
}

float LuxtronikV1Sensor::GetValue(const std::string &message) {
  return std::atof(message.c_str()) / 10;
}

void LuxtronikV1Sensor::send_cmd_(const std::string &message) {
  if (!this->uart_) {
    ESP_LOGE(TAG, "UART not initialized!");
    return;
  }

  ESP_LOGI(TAG, "Sending: %s", message.c_str());  // Changed to LOGI for better visibility
  this->uart_->write_str(message.c_str());
  this->uart_->write_byte(ASCII_CR);
  this->uart_->write_byte(ASCII_LF);
  this->uart_->flush();  // Add flush to ensure data is sent
}

void LuxtronikV1Sensor::parse_cmd_(const std::string &message) {
  if (message.empty()) {
    ESP_LOGW(TAG, "Received empty message");
    return;
  }

  ESP_LOGI(TAG, "Parsing message: '%s'", message.c_str());

  if (message.find("1100") == 0 && message.length() > 4) {
    parse_temperatures_(message);
    send_cmd_("1200");
  } else if (message.find("1200") == 0 && message.length() > 4) {
    parse_inputs_(message);
    send_cmd_("1300");
  } else if (message.find("1300") == 0 && message.length() > 4) {
    parse_outputs_(message);
    send_cmd_("1700");
  } else if (message.find("1700") == 0 && message.length() > 4) {
    parse_status_(message);
    send_cmd_("3405");
  } else if (message.find("3405") == 0 && message.length() > 4) {
    parse_heating_mode_(message);
    send_cmd_("3505");
  } else if (message.find("3505") == 0 && message.length() > 4) {
    parse_water_mode_(message);
  } else if (message.find("779") == 0) {
    ESP_LOGD(TAG, "Programming Error -> Reset Programming");
    send_cmd_("3401;0");
    send_cmd_("999");
  } else if (message.find("777") == 0) {
    ESP_LOGD(TAG, "UART is ready for input");
  } else if (message.find("993") == 0) {
    ESP_LOGD(TAG, "Value saved");
  } else if (message.find("999") == 0) {
    ESP_LOGD(TAG, "Save Value");
  }
}

void LuxtronikV1Sensor::parse_temperatures_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = {
    temp_VL_ptr, temp_RL_ptr, temp_RL_Soll_ptr, temp_Heissgas_ptr, temp_Aussen_ptr,
    temp_BW_ptr, temp_BW_Soll_ptr, temp_WQ_Ein_ptr, temp_Kaeltekreis_ptr, temp_MK1_Vorl_ptr,
    temp_MK1VL_Soll_ptr, temp_Raumstat_ptr
  };
  parse_message_(message, sensors, 5);
}

void LuxtronikV1Sensor::parse_inputs_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = {
    ein_Abtau_Soledruck_Durchfluss_ptr, ein_Sperrzeit_EVU_ptr, ein_Hochdruckpressostat_ptr,
    ein_Motorschutz_ptr, ein_Niederdruckpressostat_ptr, ein_Fremdstromanode_ptr
  };
  parse_message_(message, sensors, 5);
}

void LuxtronikV1Sensor::parse_outputs_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = {
    aus_ATV_ptr, aus_BWP_ptr, aus_FBHP_ptr, aus_HZP_ptr, aus_Mischer_1_Auf_ptr,
    aus_Mischer_1_Zu_ptr, aus_VentWP_ptr, aus_VentBrunnen_ptr, aus_Verdichter_1_ptr,
    aus_Verdichter_2_ptr, aus_ZPumpe_ptr, aus_ZWE_ptr, aus_ZWE_Stoerung_ptr
  };
  parse_message_(message, sensors, 5);
}

void LuxtronikV1Sensor::parse_status_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = {
    status_Anlagentyp_ptr, status_Softwareversion_ptr, status_Bivalenzstufe_ptr, status_Betriebszustand_ptr
  };
  parse_message_(message, sensors, 5);
}

void LuxtronikV1Sensor::parse_heating_mode_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = { modus_Heizung_ptr };
  parse_message_(message, sensors, 5);
}

void LuxtronikV1Sensor::parse_water_mode_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = { modus_Warmwasser_ptr };
  parse_message_(message, sensors, 5);
}

void LuxtronikV1Sensor::parse_message_(const std::string &message, std::vector<sensor::Sensor*> &sensors, size_t start) {
  size_t end = message.find(';', start);
  for (auto &sensor : sensors) {
    start = end + 1;
    end = message.find(';', start);
    std::string value = message.substr(start, end - start);
    if (sensor != nullptr){
        sensor->publish_state(std::atof(value.c_str()) / 10);
    }
  }
}
void LuxtronikV1Sensor::set_temp_VL(sensor::Sensor* temp_VL) {
  this->temp_VL_ptr = temp_VL;
  }
void LuxtronikV1Sensor::set_temp_RL(sensor::Sensor* temp_RL) {
    this->temp_RL_ptr = temp_RL;
  }
void LuxtronikV1Sensor::set_temp_RL_Soll(sensor::Sensor* temp_RL_Soll) {
    this->temp_RL_Soll_ptr = temp_RL_Soll;
  }
void LuxtronikV1Sensor::set_temp_Heissgas(sensor::Sensor* temp_Heissgas) {
    this->temp_Heissgas_ptr = temp_Heissgas;
  }
void LuxtronikV1Sensor::set_temp_Aussen(sensor::Sensor* temp_Aussen) {
    this->temp_Aussen_ptr = temp_Aussen;
  }
void LuxtronikV1Sensor::set_temp_BW(sensor::Sensor* temp_BW) {
    this->temp_BW_ptr = temp_BW;
  }
void LuxtronikV1Sensor::set_temp_BW_Soll(sensor::Sensor* temp_BW_Soll) {
    this->temp_BW_Soll_ptr = temp_BW_Soll;
  }
void LuxtronikV1Sensor::set_temp_WQ_Ein(sensor::Sensor* temp_WQ_Ein) {
    this->temp_WQ_Ein_ptr = temp_WQ_Ein;
  }
void LuxtronikV1Sensor::set_temp_Kaeltekreis(sensor::Sensor* temp_Kaeltekreis) {
    this->temp_Kaeltekreis_ptr = temp_Kaeltekreis;
  }
void LuxtronikV1Sensor::set_temp_MK1_Vorl(sensor::Sensor* temp_MK1_Vorl) {
    this->temp_MK1_Vorl_ptr = temp_MK1_Vorl;
  }
void LuxtronikV1Sensor::set_temp_MK1VL_Soll(sensor::Sensor* temp_MK1VL_Soll) {
    this->temp_MK1VL_Soll_ptr = temp_MK1VL_Soll;
  }
void LuxtronikV1Sensor::set_temp_Raumstat(sensor::Sensor* temp_Raumstat) {
    this->temp_Raumstat_ptr = temp_Raumstat;
  }
void LuxtronikV1Sensor::set_ein_Abtau_Soledruck_Durchfluss(sensor::Sensor* ein_Abtau_Soledruck_Durchfluss) {
    this->ein_Abtau_Soledruck_Durchfluss_ptr = ein_Abtau_Soledruck_Durchfluss;
  }
void LuxtronikV1Sensor::set_ein_Sperrzeit_EVU(sensor::Sensor* ein_Sperrzeit_EVU) {
    this->ein_Sperrzeit_EVU_ptr = ein_Sperrzeit_EVU;
  }
void LuxtronikV1Sensor::set_ein_Hochdruckpressostat(sensor::Sensor* ein_Hochdruckpressostat) {
    this->ein_Hochdruckpressostat_ptr = ein_Hochdruckpressostat;
  }
void LuxtronikV1Sensor::set_ein_Motorschutz(sensor::Sensor* ein_Motorschutz) {
    this->ein_Motorschutz_ptr = ein_Motorschutz;
  }
void LuxtronikV1Sensor::set_ein_Niederdruckpressostat(sensor::Sensor* ein_Niederdruckpressostat) {
    this->ein_Niederdruckpressostat_ptr = ein_Niederdruckpressostat;
  }
void LuxtronikV1Sensor::set_ein_Fremdstromanode(sensor::Sensor* ein_Fremdstromanode) {
    this->ein_Fremdstromanode_ptr = ein_Fremdstromanode;
  }
void LuxtronikV1Sensor::set_aus_ATV(sensor::Sensor* aus_ATV) {
    this->aus_ATV_ptr = aus_ATV;
  }
void LuxtronikV1Sensor::set_aus_BWP(sensor::Sensor* aus_BWP) {
    this->aus_BWP_ptr = aus_BWP;
  }
void LuxtronikV1Sensor::set_aus_FBHP(sensor::Sensor* aus_FBHP) {
    this->aus_FBHP_ptr = aus_FBHP;
  }
void LuxtronikV1Sensor::set_aus_HZP(sensor::Sensor* aus_HZP) {
    this->aus_HZP_ptr = aus_HZP;
  }
void LuxtronikV1Sensor::set_aus_Mischer_1_Auf(sensor::Sensor* aus_Mischer_1_Auf) {
    this->aus_Mischer_1_Auf_ptr = aus_Mischer_1_Auf;
  }
void LuxtronikV1Sensor::set_aus_Mischer_1_Zu(sensor::Sensor* aus_Mischer_1_Zu) {
    this->aus_Mischer_1_Zu_ptr = aus_Mischer_1_Zu;
  }
void LuxtronikV1Sensor::set_aus_VentWP(sensor::Sensor* aus_VentWP) {
    this->aus_VentWP_ptr = aus_VentWP;
  }
void LuxtronikV1Sensor::set_aus_VentBrunnen(sensor::Sensor* aus_VentBrunnen) {
    this->aus_VentBrunnen_ptr = aus_VentBrunnen;
  }
void LuxtronikV1Sensor::set_aus_Verdichter_1(sensor::Sensor* aus_Verdichter_1) {
    this->aus_Verdichter_1_ptr = aus_Verdichter_1;
  }
void LuxtronikV1Sensor::set_aus_Verdichter_2(sensor::Sensor* aus_Verdichter_2) {
    this->aus_Verdichter_2_ptr = aus_Verdichter_2;
  }
void LuxtronikV1Sensor::set_aus_ZPumpe(sensor::Sensor* aus_ZPumpe) {
    this->aus_ZPumpe_ptr = aus_ZPumpe;
  }
void LuxtronikV1Sensor::set_aus_ZWE(sensor::Sensor* aus_ZWE) {
    this->aus_ZWE_ptr = aus_ZWE;
  }
void LuxtronikV1Sensor::set_aus_ZWE_Stoerung(sensor::Sensor* aus_ZWE_Stoerung) {
    this->aus_ZWE_Stoerung_ptr = aus_ZWE_Stoerung;
  }
void LuxtronikV1Sensor::set_status_Anlagentyp(sensor::Sensor* status_Anlagentyp) {
    this->status_Anlagentyp_ptr = status_Anlagentyp;
  }
void LuxtronikV1Sensor::set_status_Softwareversion(sensor::Sensor* status_Softwareversion) {
    this->status_Softwareversion_ptr = status_Softwareversion;
  }
void LuxtronikV1Sensor::set_status_Bivalenzstufe(sensor::Sensor* status_Bivalenzstufe) {
    this->status_Bivalenzstufe_ptr = status_Bivalenzstufe;
  }
void LuxtronikV1Sensor::set_status_Betriebszustand(sensor::Sensor* status_Betriebszustand) {
    this->status_Betriebszustand_ptr = status_Betriebszustand;
  }
void LuxtronikV1Sensor::set_modus_Heizung(sensor::Sensor* modus_Heizung) {
    this->modus_Heizung_ptr = modus_Heizung;
  }
void LuxtronikV1Sensor::set_modus_Warmwasser(sensor::Sensor* modus_Warmwasser) {
    this->modus_Warmwasser_ptr = modus_Warmwasser;
  }

}  // namespace luxtronik_v1_sensor
}  // namespace esphome