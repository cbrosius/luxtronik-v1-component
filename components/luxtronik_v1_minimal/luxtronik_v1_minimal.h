#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace luxtronik_v1_minimal {

class LuxtronikV1MinimalSensor : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
 protected:
  void send_cmd(std::string message);
};

}  // namespace luxtronik_v1_minimal
}  // namespace esphome