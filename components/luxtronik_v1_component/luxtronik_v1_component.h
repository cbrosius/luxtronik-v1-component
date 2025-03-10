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
  LuxtronikV1Component() : PollingComponent(30000) {}  // Default to 30 seconds

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
  // Add input sensor setters
  void set_eingang_abtau_soledruck_durchfluss_sensor(sensor::Sensor *sens) { eingang_abtau_soledruck_durchfluss_ = sens; }
  void set_eingang_sperrzeit_evu_sensor(sensor::Sensor *sens) { eingang_sperrzeit_evu_ = sens; }
  void set_eingang_hochdruckpressostat_sensor(sensor::Sensor *sens) { eingang_hochdruckpressostat_ = sens; }
  void set_eingang_motorschutz_sensor(sensor::Sensor *sens) { eingang_motorschutz_ = sens; }
  void set_eingang_niederdruckpressostat_sensor(sensor::Sensor *sens) { eingang_niederdruckpressostat_ = sens; }
  void set_eingang_fremdstromanode_sensor(sensor::Sensor *sens) { eingang_fremdstromanode_ = sens; }
  // Output sensor setters
  void set_ausgang_abtauventil_sensor(sensor::Sensor *sens) { ausgang_abtauventil_ = sens; }
  void set_ausgang_brauchwasserpumpe_sensor(sensor::Sensor *sens) { ausgang_brauchwasserpumpe_ = sens; }
  void set_ausgang_fussbodenheizungspumpe_sensor(sensor::Sensor *sens) { ausgang_fussbodenheizungspumpe_ = sens; }
  void set_ausgang_heizungspumpe_sensor(sensor::Sensor *sens) { ausgang_heizungspumpe_ = sens; }
  void set_ausgang_mischer_1_auf_sensor(sensor::Sensor *sens) { ausgang_mischer_1_auf_ = sens; }
  void set_ausgang_mischer_1_zu_sensor(sensor::Sensor *sens) { ausgang_mischer_1_zu_ = sens; }
  void set_ausgang_ventilator_waermepumpe_sensor(sensor::Sensor *sens) { ausgang_ventilator_waermepumpe_ = sens; }
  void set_ausgang_ventilator_brunnen_sensor(sensor::Sensor *sens) { ausgang_ventilator_brunnen_ = sens; }
  void set_ausgang_verdichter_1_sensor(sensor::Sensor *sens) { ausgang_verdichter_1_ = sens; }
  void set_ausgang_verdichter_2_sensor(sensor::Sensor *sens) { ausgang_verdichter_2_ = sens; }
  void set_ausgang_zirkulationspumpe_sensor(sensor::Sensor *sens) { ausgang_zirkulationspumpe_ = sens; }
  void set_ausgang_zweiter_waermeerzeuger_sensor(sensor::Sensor *sens) { ausgang_zweiter_waermeerzeuger_ = sens; }
  void set_ausgang_zweiter_waermeerzeuger_stoerung_sensor(sensor::Sensor *sens) { ausgang_zweiter_waermeerzeuger_stoerung_ = sens; }

  protected:
  float get_float_temp_(const std::string& value) { return std::atof(value.c_str()) / 10.0f; }
  void parse_message_(const char* message);
  void parse_temperature_message_(const char* message);
  void parse_input_message_(const char* message);
  void parse_output_message_(const char* message);
  void publish_state_deferred_(sensor::Sensor* sensor, float value, const char* type, const char* name);

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
  sensor::Sensor *eingang_abtau_soledruck_durchfluss_{nullptr};
  // Input sensor pointers
  sensor::Sensor *eingang_sperrzeit_evu_{nullptr};
  sensor::Sensor *eingang_hochdruckpressostat_{nullptr};
  sensor::Sensor *eingang_motorschutz_{nullptr};
  sensor::Sensor *eingang_niederdruckpressostat_{nullptr};
  sensor::Sensor *eingang_fremdstromanode_{nullptr};
  // Output sensor pointers
  sensor::Sensor *ausgang_abtauventil_{nullptr};
  sensor::Sensor *ausgang_brauchwasserpumpe_{nullptr};
  sensor::Sensor *ausgang_fussbodenheizungspumpe_{nullptr};
  sensor::Sensor *ausgang_heizungspumpe_{nullptr};
  sensor::Sensor *ausgang_mischer_1_auf_{nullptr};
  sensor::Sensor *ausgang_mischer_1_zu_{nullptr};
  sensor::Sensor *ausgang_ventilator_waermepumpe_{nullptr};
  sensor::Sensor *ausgang_ventilator_brunnen_{nullptr};
  sensor::Sensor *ausgang_verdichter_1_{nullptr};
  sensor::Sensor *ausgang_verdichter_2_{nullptr};
  sensor::Sensor *ausgang_zirkulationspumpe_{nullptr};
  sensor::Sensor *ausgang_zweiter_waermeerzeuger_{nullptr};
  sensor::Sensor *ausgang_zweiter_waermeerzeuger_stoerung_{nullptr};
};

}  // namespace luxtronik_v1_component
}  // namespace esphome