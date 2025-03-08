import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

# Import the controller class from __init__.py
from . import LuxtronikV1Controller

DEPENDENCIES = ['uart']

luxtronik_v1_ns = cg.esphome_ns.namespace("luxtronik_v1")
LuxtronikV1Sensor = luxtronik_v1_ns.class_(
    "LuxtronikV1Sensor", cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        LuxtronikV1Sensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend({
         cv.Required("luxtronik_v1_id"): cv.use_id(LuxtronikV1Controller),
    })
)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    # The sensor will later receive the UART pointer from its controller.
    # Additionally, register the sensor with the controller:
    controller = await cg.get_variable(config["luxtronik_v1_id"])
    cg.add(controller.register_sensor(var))
