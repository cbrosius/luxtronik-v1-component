#include "esphome/core/log.h"
#include "luxtronik_v1_comp.h"

namespace esphome {

namespace luxtronik_v1_comp {

static const char *TAG = "luxtronik_v1_comp";

LuxtronikV1Component::LuxtronikV1Component() 
    : PollingComponent(60000), uart_(nullptr), temp_VL_ptr(nullptr),
      temp_RL_ptr(nullptr) {}

void LuxtronikV1Component::setup() {
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

void LuxtronikV1Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Luxtronik V1 Sensor:");
  LOG_SENSOR("  ", "Temperature VL", this->temp_VL_ptr);
  LOG_SENSOR("  ", "Temperature RL", this->temp_RL_ptr);
}

void LuxtronikV1Component::loop() {

  if (!this->uart_) {
    ESP_LOGE(TAG, "UART not initialized!");
    return;
  }

  // Read message
  while (available()) {
    uint8_t byte;
    this->uart_->read_byte(&byte);

    if (this->read_pos_ == READ_BUFFER_LENGTH)
      this->read_pos_ = 0;

    ESP_LOGVV(TAG, "Buffer pos: %u %d", this->read_pos_, byte);  // NOLINT

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

void LuxtronikV1Component::update() {
  // Ask for Temperatures
  send_cmd_("1100");
}

float LuxtronikV1Component::GetValue(const std::string &message) {
  return std::atof(message.c_str()) / 10;
}

void LuxtronikV1Component::send_cmd_(const std::string &message) {
  
  if (!this->uart_) {
    ESP_LOGE(TAG, "UART not initialized!");
    return;
  }

  ESP_LOGD(TAG, "S: %s - %d", message.c_str(), 0);
  this->uart_->write_str(message.c_str());
  this->uart_->write_byte(ASCII_CR);
  this->uart_->write_byte(ASCII_LF);
}

void LuxtronikV1Component::parse_cmd_(const std::string &message) {
  if (message.empty())
    return;

  ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);

  if (message.find("1100") == 0 && message.length() > 4) {
    parse_temperatures_(message);
    send_cmd_("1200");
  } 
}

void LuxtronikV1Component::parse_temperatures_(const std::string &message) {
  std::vector<sensor::Sensor*> sensors = {
    temp_VL_ptr, temp_RL_ptr
  };
  parse_message_(message, sensors, 5);
}
void LuxtronikV1Component::parse_message_(const std::string &message, std::vector<sensor::Sensor*> &sensors, size_t start) {
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
void LuxtronikV1Component::set_temp_VL(sensor::Sensor* temp_VL) {
  this->temp_VL_ptr = temp_VL;
  }
void LuxtronikV1Component::set_temp_RL(sensor::Sensor* temp_RL) {
    this->temp_RL_ptr = temp_RL;
  }

}  // namespace luxtronik_v1_comp
}  // namespace esphome
