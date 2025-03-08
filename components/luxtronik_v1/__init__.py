import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor']

CONF_LUXTRONIK_V1_ID = "luxtronik_v1_id"
CONF_TEMPERATURE_SENSORS = "temperature_sensors"

luxtronik_v1_ns = cg.esphome_ns.namespace('luxtronik_v1')
Luxtronik_v1_Component = luxtronik_v1_ns.class_('luxtronik_v1_sensor', cg.PollingComponent, uart.UARTDevice)

TEMPERATURE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT
)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Luxtronik_v1_Component),
    cv.Optional(CONF_TEMPERATURE_SENSORS): cv.ensure_list(TEMPERATURE_SENSOR_SCHEMA),
}).extend(cv.polling_component_schema('60s')).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_TEMPERATURE_SENSORS in config:
        for i, conf in enumerate(config[CONF_TEMPERATURE_SENSORS]):
            sens = await sensor.new_sensor(conf)
            cg.add(var.set_temperature_sensor(i, sens))