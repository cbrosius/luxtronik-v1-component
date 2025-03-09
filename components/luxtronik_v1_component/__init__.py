import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["uart"]
CONF_TEMPERATURE_VL = "temp_vl"
CONF_TEMPERATURE_RL = "temp_rl"

luxtronik_v1_component_ns = cg.esphome_ns.namespace("luxtronik_v1_component")
LuxtronikV1Component = luxtronik_v1_component_ns.class_(
    "LuxtronikV1Component", cg.Component, uart.UARTDevice
)

TEMPERATURE_SCHEMA = sensor.sensor_schema(
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
        cv.Optional(CONF_TEMPERATURE_VL): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_RL): TEMPERATURE_SCHEMA,
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    
    if CONF_TEMPERATURE_VL in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_VL])
        cg.add(var.set_temperature_vl_sensor(sens))
    
    if CONF_TEMPERATURE_RL in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_RL])
        cg.add(var.set_temperature_rl_sensor(sens))
