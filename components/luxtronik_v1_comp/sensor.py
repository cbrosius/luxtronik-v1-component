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


def luxtronik_v1_comp_schema():
    return sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, accuracy_decimals=1, device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT)

# Component schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
        cv.Optional(CONF_TEMP_VL): luxtronik_v1_comp_schema(),
        cv.Optional(CONF_TEMP_RL): luxtronik_v1_comp_schema(),
    }
).extend(cv.polling_component_schema("10s"))


async def to_code(config):
    # Create the object.
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # Get the uart component.
    uart_comp = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_comp))

    # Temperature Sensors
    if CONF_TEMP_VL in config:
        conf = config[CONF_TEMP_VL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temp_VL(sens))
    if CONF_TEMP_RL in config:
        conf = config[CONF_TEMP_RL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temp_RL(sens))
