#include "esphome/core/log.h"
#include "luxtronik_v1_component.h"

namespace esphome {
namespace luxtronik_v1_component {

static const char *TAG = "luxtronik_v1_component.component";

void LuxtronikV1Component::setup() {

}

void LuxtronikV1Component::loop() {

}

void LuxtronikV1Component::dump_config(){
    ESP_LOGCONFIG(TAG, "Luxtronik_v1 component");
}

}  // namespace luxtronik_v1_component
}  // namespace esphome