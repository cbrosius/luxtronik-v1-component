import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor']

luxtronik_v1_ns = cg.esphome_ns.namespace('luxtronik_v1')
Luxtronik_v1_Component = luxtronik_v1_ns.class_('luxtronik_v1_sensor', cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Luxtronik_v1_Component)
}).extend(cv.polling_component_schema('60s')).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)