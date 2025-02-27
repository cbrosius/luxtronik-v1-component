#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"
#include <vector>
#include <memory>

namespace esphome {

namespace luxtronik_v1_comp {

const char ASCII_CR = 0x0D;
const char ASCII_LF = 0x0A;

const uint8_t READ_BUFFER_LENGTH = 255;

class LuxtronikV1Component : public PollingComponent, public uart::UARTDevice {
 public:
  LuxtronikV1Component();
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  void set_uart(uart::UARTComponent *uart){
      this->uart_ = uart;
  }
  void set_temp_VL(sensor::Sensor* temp_VL);
  void set_temp_RL(sensor::Sensor* temp_RL);

 protected:
  uart::UARTComponent* uart_;
  std::string sender_;
  char read_buffer_[READ_BUFFER_LENGTH];
  size_t read_pos_{0};

  float GetValue(const std::string &message);
  void send_cmd_(const std::string &message);
  void parse_cmd_(const std::string &message);
  void parse_temperatures_(const std::string &message);
  void parse_inputs_(const std::string &message);
  void parse_outputs_(const std::string &message);
  void parse_status_(const std::string &message);
  void parse_heating_mode_(const std::string &message);
  void parse_water_mode_(const std::string &message);
  void parse_message_(const std::string &message, std::vector<sensor::Sensor*> &sensors, size_t start);

  // Sensor pointers
  sensor::Sensor *temp_VL_ptr;
  sensor::Sensor *temp_RL_ptr;

  CallbackManager<void(std::string, std::string)> callback_;
};

}  // namespace luxtronik_v1_comp
}  // namespace esphome
