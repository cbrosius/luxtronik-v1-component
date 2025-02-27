import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

luxtronik_v1_ns = cg.esphome_ns.namespace("luxtronik_v1_comp")
LuxtronikV1Component = luxtronik_v1_ns.class_(
    "LuxtronikV1Component", cg.PollingComponent, uart.UARTDevice
)

DEPENDENCIES = ["uart"]

CONF_UART_ID = "uart_id"
CONF_TEMP_VL = "temp_VL"
CONF_TEMP_RL = "temp_RL"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_TEMP_VL): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TEMP_RL): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(cv.polling_component_schema("10s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    uart_comp = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_comp))

    if CONF_TEMP_VL in config:
        sens = await sensor.new_sensor(config[CONF_TEMP_VL])
        cg.add(var.set_temp_VL(sens))
    if CONF_TEMP_RL in config:
        sens = await sensor.new_sensor(config[CONF_TEMP_RL])
        cg.add(var.set_temp_RL(sens))
