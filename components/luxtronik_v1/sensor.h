#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace luxtronik_v1 {

class LuxtronikV1Sensor : public PollingComponent, public uart::UARTDevice {
 public:
  LuxtronikV1Sensor();  // Changed class name to match Python code
  
  void set_uart(uart::UARTComponent *uart);
  void loop() override;
  void update() override;
  bool register_sensor(sensor::Sensor *sens);

  // Temperature Sensors
  sensor::Sensor *temp_VL{nullptr};           // Temperatur Vorlauf
  sensor::Sensor *temp_RL{nullptr};           // Temperatur Rücklauf
  sensor::Sensor *temp_RL_Soll{nullptr};      // Temperatur Rücklauf-Soll
  sensor::Sensor *temp_Heissgas{nullptr};     // Heissgas Temperature
  sensor::Sensor *temp_Aussen{nullptr};       // Outside Temperature
  sensor::Sensor *temp_BW{nullptr};           // Brauchwasser Temperature
  sensor::Sensor *temp_BW_Soll{nullptr};      // Brauchwasser Soll Temperature
  sensor::Sensor *temp_WQ_Ein{nullptr};       // Wärmequelle Input Temperature
  sensor::Sensor *temp_Kaeltekreis{nullptr};  // Kältekreis Temperature
  sensor::Sensor *temp_MK1_Vorl{nullptr};     // Mischkreis 1 Vorlauf
  sensor::Sensor *temp_MK1VL_Soll{nullptr};   // Mischkreis 1 Vorlauf Soll
  sensor::Sensor *temp_Raumstat{nullptr};     // Raumstation Temperature

  // Input Sensors
  sensor::Sensor *ein_Abtau_Soledruck_Durchfluss{nullptr};
  sensor::Sensor *ein_Sperrzeit_EVU{nullptr};
  sensor::Sensor *ein_Hochdruckpressostat{nullptr};
  sensor::Sensor *ein_Motorschutz{nullptr};
  sensor::Sensor *ein_Niederdruckpressostat{nullptr};
  sensor::Sensor *ein_Fremdstromanode{nullptr};

  // Output Sensors
  sensor::Sensor *aus_ATV{nullptr};          // Abtauventil
  sensor::Sensor *aus_BWP{nullptr};          // Brauchwasserpumpe
  sensor::Sensor *aus_FBHP{nullptr};         // Fussbodenheizungspumpe
  sensor::Sensor *aus_HZP{nullptr};          // Heizungspumpe
  sensor::Sensor *aus_Mischer_1_Auf{nullptr};
  sensor::Sensor *aus_Mischer_1_Zu{nullptr};
  sensor::Sensor *aus_VentWP{nullptr};       // Ventilation Wärmepumpe
  sensor::Sensor *aus_VentBrunnen{nullptr};  // Ventilator/Brunnen
  sensor::Sensor *aus_Verdichter_1{nullptr};
  sensor::Sensor *aus_Verdichter_2{nullptr};
  sensor::Sensor *aus_ZPumpe{nullptr};       // Zusatzpumpe
  sensor::Sensor *aus_ZWE{nullptr};          // Zweiter Wärmeerzeuger
  sensor::Sensor *aus_ZWE_Stoerung{nullptr}; // ZWE Störung

  // State Sensors
  sensor::Sensor *state_Anlagentyp{nullptr};
  sensor::Sensor *state_Softwareversion{nullptr};
  sensor::Sensor *state_Bivalenzstufe{nullptr};
  sensor::Sensor *state_Betriebszustand{nullptr};

  // Status Sensors
  sensor::Sensor *status_StartDate_Day{nullptr};
  sensor::Sensor *status_StartDate_Month{nullptr};
  sensor::Sensor *status_StartDate_Year{nullptr};
  sensor::Sensor *status_StartTime_Hour{nullptr};
  sensor::Sensor *status_StartTime_Min{nullptr};
  sensor::Sensor *status_StartTime_Sec{nullptr};
  sensor::Sensor *status_Compact{nullptr};
  
 protected:
  uart::UARTComponent *uart_{nullptr};
  std::string sender_;
  char read_buffer_[255];
  size_t read_pos_{0};
  void send_cmd_(std::string message);
  void parse_cmd_(std::string message);
  float GetFloatTemp(std::string message);
  float GetInputOutputState(std::string message);
};

}  // namespace luxtronik_v1
}  // namespace esphome