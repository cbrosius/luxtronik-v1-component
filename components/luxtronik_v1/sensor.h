#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace luxtronik_v1 {

class luxtronik_v1_sensor : public PollingComponent, public uart::UARTDevice {
 public:
  luxtronik_v1_sensor(UARTComponent *parent);
  void loop() override;
  void update() override;

  // Temperature Sensors
  Sensor *temp_VL{nullptr};           // Temperatur Vorlauf
  Sensor *temp_RL{nullptr};           // Temperatur Rücklauf
  Sensor *temp_RL_Soll{nullptr};      // Temperatur Rücklauf-Soll
  Sensor *temp_Heissgas{nullptr};     // Heissgas Temperature
  Sensor *temp_Aussen{nullptr};       // Outside Temperature
  Sensor *temp_BW{nullptr};           // Brauchwasser Temperature
  Sensor *temp_BW_Soll{nullptr};      // Brauchwasser Soll Temperature
  Sensor *temp_WQ_Ein{nullptr};       // Wärmequelle Input Temperature
  Sensor *temp_Kaeltekreis{nullptr};  // Kältekreis Temperature
  Sensor *temp_MK1_Vorl{nullptr};     // Mischkreis 1 Vorlauf
  Sensor *temp_MK1VL_Soll{nullptr};   // Mischkreis 1 Vorlauf Soll
  Sensor *temp_Raumstat{nullptr};     // Raumstation Temperature

  // Input Sensors
  Sensor *ein_Abtau_Soledruck_Durchfluss{nullptr};
  Sensor *ein_Sperrzeit_EVU{nullptr};
  Sensor *ein_Hochdruckpressostat{nullptr};
  Sensor *ein_Motorschutz{nullptr};
  Sensor *ein_Niederdruckpressostat{nullptr};
  Sensor *ein_Fremdstromanode{nullptr};

  // Output Sensors
  Sensor *aus_ATV{nullptr};          // Abtauventil
  Sensor *aus_BWP{nullptr};          // Brauchwasserpumpe
  Sensor *aus_FBHP{nullptr};         // Fussbodenheizungspumpe
  Sensor *aus_HZP{nullptr};          // Heizungspumpe
  Sensor *aus_Mischer_1_Auf{nullptr};
  Sensor *aus_Mischer_1_Zu{nullptr};
  Sensor *aus_VentWP{nullptr};       // Ventilation Wärmepumpe
  Sensor *aus_VentBrunnen{nullptr};  // Ventilator/Brunnen
  Sensor *aus_Verdichter_1{nullptr};
  Sensor *aus_Verdichter_2{nullptr};
  Sensor *aus_ZPumpe{nullptr};       // Zusatzpumpe
  Sensor *aus_ZWE{nullptr};          // Zweiter Wärmeerzeuger
  Sensor *aus_ZWE_Stoerung{nullptr}; // ZWE Störung

  // State Sensors
  Sensor *state_Anlagentyp{nullptr};
  Sensor *state_Softwareversion{nullptr};
  Sensor *state_Bivalenzstufe{nullptr};
  Sensor *state_Betriebszustand{nullptr};

  // Status Sensors
  Sensor *status_StartDate_Day{nullptr};
  Sensor *status_StartDate_Month{nullptr};
  Sensor *status_StartDate_Year{nullptr};
  Sensor *status_StartTime_Hour{nullptr};
  Sensor *status_StartTime_Min{nullptr};
  Sensor *status_StartTime_Sec{nullptr};
  Sensor *status_Compact{nullptr};
  
 protected:
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