import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import ICON_EMPTY, UNIT_EMPTY

DEPENDENCIES = ["uart"]

luxtronik_v1_minimal_ns = cg.esphome_ns.namespace("luxtronik_v1_minimal")
LuxtronikV1MinimalSensor = luxtronik_v1_minimal_ns.class_(
    "LuxtronikV1MinimalSensor", cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        LuxtronikV1MinimalSensor,
        unit_of_measurement=UNIT_EMPTY,
        icon=ICON_EMPTY,
        accuracy_decimals=1,
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
