#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"

namespace esphome {
namespace luxtronik_v1_minimal {

class LuxtronikV1MinimalSensor : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
  // Constructor with polling interval
  LuxtronikV1MinimalSensor() : PollingComponent(5000) {}

  void set_uart_parent(uart::UARTComponent *parent) { 
    this->parent_ = parent;  // Store UART parent
  }

  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;

 protected:
  void send_cmd(std::string message);
  uint32_t last_read_{0};
  uart::UARTComponent *parent_{nullptr};  // Add parent pointer
};

}  // namespace luxtronik_v1_minimal
}  // namespace esphome