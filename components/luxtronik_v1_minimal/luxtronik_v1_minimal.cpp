#include "esphome/core/log.h"
#include "luxtronik_v1_minimal.h"

namespace esphome {
namespace luxtronik_v1_minimal {

static const char *TAG = "luxtronik_v1_minimal.sensor";

void LuxtronikV1MinimalSensor::setup() {

}

void LuxtronikV1MinimalSensor::update() {
    // Ask for Temperatures
    send_cmd("1100");
}

void LuxtronikV1MinimalSensor::loop() {

}

void LuxtronikV1MinimalSensor::dump_config(){
    ESP_LOGCONFIG(TAG, "LuxtronikV1MinimalSensor");
}

void LuxtronikV1MinimalSensor::send_cmd(std::string message) {
    if (this->uart_ == nullptr) {
        ESP_LOGW(TAG, "send_cmd_() - UART not set");
        return;
    }
    
    ESP_LOGV(TAG, "Sending command: %s", message.c_str());
    
    // Add CR+LF to the message
    message += "\r\n";
    
    // Write the command to UART
    this->write_str(message.c_str());
}

}  // namespace luxtronik_v1_minimal
}  // namespace esphome