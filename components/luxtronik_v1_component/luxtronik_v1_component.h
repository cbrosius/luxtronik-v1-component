#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace luxtronik_v1_component {

static const char ASCII_CR = '\r';
static const char ASCII_LF = '\n';
static const uint8_t READ_BUFFER_LENGTH = 255;

class LuxtronikV1Component : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;

  void set_uart_parent(uart::UARTComponent *parent) { 
    this->parent_ = parent;
  }

  // Add temperature sensor setters
  void set_temperature_vorlauf_sensor(sensor::Sensor *sens) { temperature_vorlauf_ = sens; }
  void set_temperature_ruecklauf_sensor(sensor::Sensor *sens) { temperature_ruecklauf_ = sens; }
  void set_temperature_ruecklauf_soll_sensor(sensor::Sensor *sens) { temperature_ruecklauf_soll_ = sens; }
  void set_temperature_heissgas_sensor(sensor::Sensor *sens) { temperature_heissgas_ = sens; }
  void set_temperature_aussen_sensor(sensor::Sensor *sens) { temperature_aussen_ = sens; }
  void set_temperature_brauchwasser_sensor(sensor::Sensor *sens) { temperature_brauchwasser_ = sens; }
  void set_temperature_brauchwasser_soll_sensor(sensor::Sensor *sens) { temperature_brauchwasser_soll_ = sens; }
  void set_temperature_waermequelle_eingang_sensor(sensor::Sensor *sens) { temperature_waermequelle_eingang_ = sens; }
  void set_temperature_kaeltekreis_sensor(sensor::Sensor *sens) { temperature_kaeltekreis_ = sens; }
  void set_temperature_mischkreis1_vorlauf_sensor(sensor::Sensor *sens) { temperature_mischkreis1_vorlauf_ = sens; }
  void set_temperature_mischkreis1_vorlauf_soll_sensor(sensor::Sensor *sens) { temperature_mischkreis1_vorlauf_soll_ = sens; }
  void set_temperature_raumstation_sensor(sensor::Sensor *sens) { temperature_raumstation_ = sens; }

 protected:
  void parse_message_(const char* message);
  float get_float_temp_(const std::string& value) { return std::atof(value.c_str()) / 10.0f; }

  uart::UARTComponent *parent_{nullptr};
  char read_buffer_[READ_BUFFER_LENGTH];
  size_t read_pos_{0};

  // Temperature sensor pointers
  sensor::Sensor *temperature_vorlauf_{nullptr};  // Vorlauf Temperatur
  sensor::Sensor *temperature_ruecklauf_{nullptr};  // Rücklauf Temperatur
  sensor::Sensor *temperature_ruecklauf_soll_{nullptr};
  sensor::Sensor *temperature_heissgas_{nullptr};
  sensor::Sensor *temperature_aussen_{nullptr};
  sensor::Sensor *temperature_brauchwasser_{nullptr};
  sensor::Sensor *temperature_brauchwasser_soll_{nullptr};
  sensor::Sensor *temperature_waermequelle_eingang_{nullptr};
  sensor::Sensor *temperature_kaeltekreis_{nullptr};
  sensor::Sensor *temperature_mischkreis1_vorlauf_{nullptr};
  sensor::Sensor *temperature_mischkreis1_vorlauf_soll_{nullptr};
  sensor::Sensor *temperature_raumstation_{nullptr};
};

}  // namespace luxtronik_v1_component
}  // namespace esphome