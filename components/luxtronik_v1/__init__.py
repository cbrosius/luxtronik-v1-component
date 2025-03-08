import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor']

# Configuration constants
CONF_LUXTRONIK_V1_ID = "luxtronik_v1_id"
CONF_TEMP_VL = "temp_VL"
CONF_TEMP_RL = "temp_RL"

luxtronik_v1_ns = cg.esphome_ns.namespace('luxtronik_v1')
LuxtronikV1Component = luxtronik_v1_ns.class_('luxtronik_v1_sensor', cg.PollingComponent, uart.UARTDevice)

# Schema for individual temperature sensors
TEMPERATURE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_TEMP_VL): TEMPERATURE_SCHEMA,
    cv.Optional(CONF_TEMP_RL): TEMPERATURE_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA).extend(sensor.sensor_schema())

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_component))
    await uart.register_uart_device(var, config)

    if CONF_TEMP_VL in config:
        conf = config[CONF_TEMP_VL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temp_VL(sens))
    
    if CONF_TEMP_RL in config:
        conf = config[CONF_TEMP_RL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temp_RL(sens))