#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"

namespace esphome {
namespace luxtronik_v1_minimal {

static const uint8_t READ_BUFFER_LENGTH = 255;
static const char ASCII_CR = '\r';
static const char ASCII_LF = '\n';

class LuxtronikV1MinimalSensor : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
  // Constructor with polling interval
  LuxtronikV1MinimalSensor() : PollingComponent() {}

  void set_uart_parent(uart::UARTComponent *parent) { 
    this->parent_ = parent;
  }

  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;

 protected:
  void send_cmd(std::string message);
  
  // Buffer management
  char read_buffer_[READ_BUFFER_LENGTH];
  size_t read_pos_{0};
  
  // UART parent
  uart::UARTComponent *parent_{nullptr};
};

}  // namespace luxtronik_v1_minimal
}  // namespace esphome