#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace luxtronik_v1_component {

static const char ASCII_CR = '\r';
static const char ASCII_LF = '\n';
static const uint8_t READ_BUFFER_LENGTH = 255;

class LuxtronikV1Component : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_uart_parent(uart::UARTComponent *parent) { 
    this->parent_ = parent;
  }

 protected:
  uart::UARTComponent *parent_{nullptr};
  char read_buffer_[READ_BUFFER_LENGTH];
  size_t read_pos_{0};
};

}  // namespace luxtronik_v1_component
}  // namespace esphome