#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace luxtronik_v1_component {

static const char ASCII_CR = '\r';
static const char ASCII_LF = '\n';
static const uint8_t READ_BUFFER_LENGTH = 255;

class LuxtronikV1Component : public uart::UARTDevice, public PollingComponent {
 public:
  LuxtronikV1Component() : PollingComponent(10000) {}  // Default to 10 seconds

  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;

  void set_uart_parent(uart::UARTComponent *parent) { 
    this->parent_ = parent;
  }

  // Add temperature sensor setters
  void set_temperature_vl_sensor(sensor::Sensor *sens) { temperature_vl_ = sens; }
  void set_temperature_rl_sensor(sensor::Sensor *sens) { temperature_rl_ = sens; }

 protected:
  void parse_message_(const char* message);
  float get_float_temp_(const std::string& value) { return std::atof(value.c_str()) / 10.0f; }

  uart::UARTComponent *parent_{nullptr};
  char read_buffer_[READ_BUFFER_LENGTH];
  size_t read_pos_{0};

  // Temperature sensor pointers
  sensor::Sensor *temperature_vl_{nullptr};  // Vorlauf Temperatur
  sensor::Sensor *temperature_rl_{nullptr};  // Rücklauf Temperatur
};

}  // namespace luxtronik_v1_component
}  // namespace esphome