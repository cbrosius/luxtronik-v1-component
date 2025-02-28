#include "esphome.h"
#include "sensor.h"

namespace esphome {
namespace luxtronik_v1 {

const char ASCII_CR = 0x0D;
const char ASCII_LF = 0x0A;
const uint8_t READ_BUFFER_LENGTH = 255;
static const char* TAG = "luxtronik_v1";

// Constructor implementation
luxtronik_v1_sensor::luxtronik_v1_sensor(UARTComponent *parent) 
    : PollingComponent(60000), UARTDevice(parent) {
    // Initialize sensors
    temp_VL = new Sensor();
    temp_RL = new Sensor();
    temp_RL_Soll = new Sensor();
    temp_Heissgas = new Sensor();
    temp_Aussen = new Sensor();
    temp_BW = new Sensor();
    temp_BW_Soll = new Sensor();
    temp_WQ_Ein = new Sensor();
    temp_Kaeltekreis = new Sensor();
    temp_MK1_Vorl = new Sensor();
    temp_MK1VL_Soll = new Sensor();
    temp_Raumstat = new Sensor();
    ein_Abtau_Soledruck_Durchfluss = new Sensor();
    ein_Sperrzeit_EVU = new Sensor();
    ein_Hochdruckpressostat = new Sensor();
    ein_Motorschutz = new Sensor();
    ein_Niederdruckpressostat = new Sensor();
    ein_Fremdstromanode = new Sensor();
    aus_ATV = new Sensor();
    aus_BWP = new Sensor();
    aus_FBHP = new Sensor();
    aus_HZP = new Sensor();
    aus_Mischer_1_Auf = new Sensor();
    aus_Mischer_1_Zu = new Sensor();
    aus_VentWP = new Sensor();
    aus_VentBrunnen = new Sensor();
    aus_Verdichter_1 = new Sensor();
    aus_Verdichter_2 = new Sensor();
    aus_ZPumpe = new Sensor();
    aus_ZWE = new Sensor();
    aus_ZWE_Stoerung = new Sensor();
    state_Anlagentyp = new Sensor();
    state_Softwareversion = new Sensor();
    state_Bivalenzstufe = new Sensor();
    state_Betriebszustand = new Sensor();
    status_StartDate_Day = new Sensor();
    status_StartDate_Month = new Sensor();
    status_StartDate_Year = new Sensor();
    status_StartTime_Hour = new Sensor();
    status_StartTime_Min = new Sensor();
    status_StartTime_Sec = new Sensor();
    status_Compact = new Sensor();
  }

  void loop() override{
    // Read message
    while (this->available()) {
        uint8_t byte;
        this->read_byte(&byte);

        if (this->read_pos_ == READ_BUFFER_LENGTH)
        this->read_pos_ = 0;

        ESP_LOGVV(TAG, "Buffer pos: %u %d", this->read_pos_, byte);  // NOLINT

        if (byte == ASCII_CR)
        continue;
        if (byte >= 0x7F)
        byte = '?';  // need to be valid utf8 string for log functions.
        this->read_buffer_[this->read_pos_] = byte;

        if (this->read_buffer_[this->read_pos_] == ASCII_LF) {
        this->read_buffer_[this->read_pos_] = 0;
        this->read_pos_ = 0;
        this->parse_cmd_(this->read_buffer_);
        } else {
        this->read_pos_++;
        }
    }
  }

  void update() override {
    // Consider adding error handling
    if (!this->available()) {
        ESP_LOGW(TAG, "UART not available");
        return;
    }
    // Ask for Temperatures
    send_cmd_("1100"); 
  }

  float GetFloatTemp(std::string message){
    return std::atof(message.c_str())/10;
  }

  float GetInputOutputState(std::string message){
    return std::atoi(message.c_str());
  }

  void send_cmd_(std::string message){
    ESP_LOGV(TAG, "S: %s - %d", message.c_str(), 0);
    this->write_str(message.c_str());
    this->write_byte(ASCII_CR);
    this->write_byte(ASCII_LF);
  }

  void parse_cmd_(std::string message){
    // Move this check to the beginning
    if (message.empty()) {
        ESP_LOGW(TAG, "Received empty message");
        return;
    }
    
    ESP_LOGV(TAG, "R: %s - %d", message.c_str(), 0);
    std::string delimiter = ";";

    if (message.find("1100")==0){
        ESP_LOGD(TAG, "1100 found -> Temperatures");
        // 1100;12;254;257;261;456;128;480;470;177;201;750;0;0
        size_t start = 5;
        size_t end = message.find(';', start);
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_VL           = new Sensor();     // Temperatur Vorlauf  -> 1100/2 
        std::string tmp_VL = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_VL.c_str());
        temp_VL->publish_state(GetFloatTemp(tmp_VL));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_RL           = new Sensor();     // Temperatur Rücklauf -> 1100/3 
        std::string tmp_RL = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_RL.c_str());
        temp_RL->publish_state(GetFloatTemp(tmp_RL));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_RL_Soll      = new Sensor();     // Temperatur Rücklauf-Soll -> 1100/4 
        std::string tmp_RL_Soll = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_RL_Soll.c_str());
        temp_RL_Soll->publish_state(GetFloatTemp(tmp_RL_Soll));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Heissgas     = new Sensor();  // 1100/5 
        std::string tmp_Heissgas = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Heissgas.c_str());
        temp_Heissgas->publish_state(GetFloatTemp(tmp_Heissgas));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Aussen       = new Sensor();  // 1100/6 
        std::string tmp_Aussen = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Aussen.c_str());
        temp_Aussen->publish_state(GetFloatTemp(tmp_Aussen));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_BW       = new Sensor();  // 1100/7 
        std::string tmp_BW = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_BW.c_str());
        temp_BW->publish_state(GetFloatTemp(tmp_BW));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_BW_Soll      = new Sensor();  // 1100/8 
        std::string tmp_BW_Soll = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_BW_Soll.c_str());
        temp_BW_Soll->publish_state(GetFloatTemp(tmp_BW_Soll));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_WQ_Ein       = new Sensor();  // 1100/9 
        std::string tmp_WQ_Ein = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_WQ_Ein.c_str());
        temp_WQ_Ein->publish_state(GetFloatTemp(tmp_WQ_Ein));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Kaeltekreis  = new Sensor();  // 1100/10
        std::string tmp_Kaeltekreis = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Kaeltekreis.c_str());
        temp_Kaeltekreis->publish_state(GetFloatTemp(tmp_Kaeltekreis));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_MK1_Vorl     = new Sensor();  // 1100/11
        std::string tmp_MK1_Vorl = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_MK1_Vorl.c_str());
        temp_MK1_Vorl->publish_state(GetFloatTemp(tmp_MK1_Vorl));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_MK1VL_Soll   = new Sensor();  // 1100/12
        std::string tmp_MK1VL_Soll = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_MK1VL_Soll.c_str());
        temp_MK1VL_Soll->publish_state(GetFloatTemp(tmp_MK1VL_Soll));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Raumstat     = new Sensor();  // 1100/13
        std::string tmp_Raumstat = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Raumstat.c_str());
        temp_Raumstat->publish_state(GetFloatTemp(tmp_Raumstat));

        // Nach den Temperaturen die Eingänge abfragen
        send_cmd_("1200"); 
    }

    if (message.find("1200")==0){
        ESP_LOGD(TAG, "1200 found -> Inputs");
        // 1200;6;1;1;0;1;1;0
        std::string tmp_in;
        size_t start = 5;
        size_t end = message.find(';', start);
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Abtau_Soledruck_Durchfluss  = new Sensor();  // 1200/2
        tmp_in = message.substr(start, end - start).c_str();
        ein_Abtau_Soledruck_Durchfluss->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Sperrzeit_EVU               = new Sensor();  // 1200/3
        tmp_in = message.substr(start, end - start).c_str();
        ein_Sperrzeit_EVU->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Hochdruckpressostat         = new Sensor();  // 1200/4
        tmp_in = message.substr(start, end - start).c_str();
        ein_Hochdruckpressostat->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Motorschutz                 = new Sensor();  // 1200/5
        tmp_in = message.substr(start, end - start).c_str();
        ein_Motorschutz->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Niederdruckpressostat       = new Sensor();  // 1200/6
        tmp_in = message.substr(start, end - start).c_str();
        ein_Niederdruckpressostat->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Fremdstromanode             = new Sensor();  // 1200/7
        tmp_in = message.substr(start, end - start).c_str();
        ein_Fremdstromanode->publish_state(GetInputOutputState(tmp_in));

        // Nach den Eingängen die Ausgänge abfragen
        send_cmd_("1300"); 
    }

    if (message.find("1300")==0){
        ESP_LOGD(TAG, "1300 found -> Outputs");
        // 1300;13;0;0;1;1;0;1;0;0;0;0;0;0;0
        size_t start = 5;
        size_t end = message.find(';', start);
        std::string tmp_out;
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ATV             = new Sensor();  // 1300/2
        tmp_out = message.substr(start, end - start).c_str();
        aus_ATV->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_BWP             = new Sensor();  // 1300/3
        tmp_out = message.substr(start, end - start).c_str();
        aus_BWP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_FBHP            = new Sensor();  // 1300/4
        tmp_out = message.substr(start, end - start).c_str();
        aus_FBHP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_HZP             = new Sensor();  // 1300/5
        tmp_out = message.substr(start, end - start).c_str();
        aus_HZP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Mischer_1_Auf   = new Sensor();  // 1300/6
        tmp_out = message.substr(start, end - start).c_str();
        aus_Mischer_1_Auf->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Mischer_1_Zu    = new Sensor();  // 1300/7
        tmp_out = message.substr(start, end - start).c_str();
        aus_Mischer_1_Zu->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_VentWP          = new Sensor();  // 1300/8
        tmp_out = message.substr(start, end - start).c_str();
        aus_VentWP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_VentBrunnen     = new Sensor();  // 1300/9
        tmp_out = message.substr(start, end - start).c_str();
        aus_VentBrunnen->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Verdichter_1    = new Sensor();  // 1300/10
        tmp_out = message.substr(start, end - start).c_str();
        aus_Verdichter_1->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Verdichter_2    = new Sensor();  // 1300/11
        tmp_out = message.substr(start, end - start).c_str();
        aus_Verdichter_2->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ZPumpe          = new Sensor();  // 1300/12
        tmp_out = message.substr(start, end - start).c_str();
        aus_ZPumpe->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ZWE             = new Sensor();  // 1300/13
        tmp_out = message.substr(start, end - start).c_str();
        aus_ZWE->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ZWE_Stoerung    = new Sensor();  // 1300/14
        tmp_out = message.substr(start, end - start).c_str();
        aus_ZWE_Stoerung->publish_state(GetInputOutputState(tmp_out));
    }
  }


}  // namespace luxtronik_v1
}  // namespace esphome
