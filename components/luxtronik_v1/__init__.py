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

luxtronik_v1_ns = cg.esphome_ns.namespace('luxtronik_v1')
LuxtronikV1Component = luxtronik_v1_ns.class_('luxtronik_v1_sensor', cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_component))
    await uart.register_uart_device(var, config)