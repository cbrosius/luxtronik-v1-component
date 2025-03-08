import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UART_ID

DEPENDENCIES = ['uart']

luxtronik_v1_ns = cg.esphome_ns.namespace("luxtronik_v1")
# Define a controller class for the luxtronik_v1 component
LuxtronikV1Controller = luxtronik_v1_ns.class_("LuxtronikV1Controller", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LuxtronikV1Controller),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
})

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    uart_obj = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_obj))