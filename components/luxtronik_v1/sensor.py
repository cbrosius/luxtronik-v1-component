import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ['uart']

# Namespace entspricht dem Ordnernamen
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
)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    # Hier wird kein eigener uart.register_uart_device aufgerufen,
    # denn das UART wird vom Controller bereitgestellt.
