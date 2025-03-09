#include "esphome/core/log.h"
#include "luxtronik_v1_minimal.h"

namespace esphome {
namespace luxtronik_v1_minimal {

static const char *TAG = "luxtronik_v1_minimal.sensor";

void LuxtronikV1MinimalSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Luxtronik V1 Minimal...");
}

void LuxtronikV1MinimalSensor::update() {
    // Ask for Temperatures
    send_cmd("1100");
}

void LuxtronikV1MinimalSensor::loop() {
    const uint32_t now = millis();
    if (now - this->last_read_ < 5000) {
        return;
    }
    this->last_read_ = now;
}

void LuxtronikV1MinimalSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "LuxtronikV1MinimalSensor:");
}

void LuxtronikV1MinimalSensor::send_cmd(std::string message) {
    ESP_LOGV(TAG, "Sending command: %s", message.c_str());
    
    // Add CR+LF to the message
    message += "\r\n";
    
    // Write the command to UART using UARTDevice's method
    this->write_str(message.c_str());
}

}  // namespace luxtronik_v1_minimal
}  // namespace esphome