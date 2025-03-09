#include "esphome/core/log.h"
#include "luxtronik_v1_minimal.h"

namespace esphome {
namespace luxtronik_v1_minimal {

static const char *TAG = "luxtronik_v1_minimal.sensor";

void LuxtronikV1MinimalSensor::setup() {

}

void LuxtronikV1MinimalSensor::update() {

}

void LuxtronikV1MinimalSensor::loop() {

}

void LuxtronikV1MinimalSensor::dump_config(){
    ESP_LOGCONFIG(TAG, "LuxtronikV1MinimalSensor");
}

}  // namespace luxtronik_v1_minimal
}  // namespace esphome