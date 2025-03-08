import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
from . import luxtronik_v1_ns, Luxtronik_v1_Component

LuxtronikV1Sensor = luxtronik_v1_ns.class_("luxtronik_v1_sensor", sensor.Sensor)

CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
).extend({
    cv.GenerateID(): cv.declare_id(LuxtronikV1Sensor),
})

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)