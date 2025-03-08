import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
from . import luxtronik_v1_ns, LuxtronikV1Component

CONF_TEMPERATURE_SENSORS = "temperature_sensors"

CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

async def to_code(config):
    paren = await cg.get_variable(config[CONF_TEMPERATURE_SENSORS])
    var = await sensor.new_sensor(config)
    cg.add(paren.set_temperature_sensor(var))