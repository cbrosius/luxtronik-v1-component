import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["uart"]
CONF_TEMPERATURE_VORLAUF = "temperature_vorlauf"
CONF_TEMPERATURE_RUECKLAUF = "temperature_ruecklauf"
CONF_TEMPERATURE_RUECKLAUF_SOLL = "temperature_ruecklauf_soll"
CONF_TEMPERATURE_HEISSGAS = "temperature_heissgas"
CONF_TEMPERATURE_AUSSEN = "temperature_aussen"
CONF_TEMPERATURE_BRAUCHWASSER = "temperature_brauchwasser"
CONF_TEMPERATURE_BRAUCHWASSER_SOLL = "temperature_brauchwasser_soll"
CONF_TEMPERATURE_WAERMEQUELLE_EINGANG = "temperature_waermequelle_eingang"
CONF_TEMPERATURE_KAELTEKREIS = "temperature_kaeltekreis"
CONF_TEMPERATURE_mischkreis1_VORLAUF = "temperature_mischkreis1_vorlauf"
CONF_TEMPERATURE_mischkreis1_VORLAUF_SOLL = "temperature_mischkreis1_vorlauf_soll"
CONF_TEMPERATURE_raumstation = "temperature_raumstation"

luxtronik_v1_component_ns = cg.esphome_ns.namespace("luxtronik_v1_component")
LuxtronikV1Component = luxtronik_v1_component_ns.class_(
    "LuxtronikV1Component", cg.Component, uart.UARTDevice
)

TEMPERATURE_SCHEMA = sensor.sensor_schema(
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
        cv.Optional(CONF_TEMPERATURE_VORLAUF): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_RUECKLAUF): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_RUECKLAUF_SOLL): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_HEISSGAS): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_AUSSEN): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_BRAUCHWASSER): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_BRAUCHWASSER_SOLL): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_WAERMEQUELLE_EINGANG): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_KAELTEKREIS): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_mischkreis1_VORLAUF): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_mischkreis1_VORLAUF_SOLL): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_raumstation): TEMPERATURE_SCHEMA,
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    
    if CONF_TEMPERATURE_VORLAUF in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_VORLAUF])
        cg.add(var.set_temperature_vorlauf_sensor(sens))
    
    if CONF_TEMPERATURE_RUECKLAUF in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_RUECKLAUF])
        cg.add(var.set_temperature_ruecklauf_sensor(sens))

    if CONF_TEMPERATURE_RUECKLAUF_SOLL in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_RUECKLAUF_SOLL])
        cg.add(var.set_temperature_ruecklauf_soll_sensor(sens))

    if CONF_TEMPERATURE_HEISSGAS in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_HEISSGAS])
        cg.add(var.set_temperature_heissgas_sensor(sens))

    if CONF_TEMPERATURE_AUSSEN in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_AUSSEN])
        cg.add(var.set_temperature_aussen_sensor(sens))

    if CONF_TEMPERATURE_BRAUCHWASSER in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_BRAUCHWASSER])
        cg.add(var.set_temperature_brauchwasser_sensor(sens))

    if CONF_TEMPERATURE_BRAUCHWASSER_SOLL in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_BRAUCHWASSER_SOLL])
        cg.add(var.set_temperature_brauchwasser_soll_sensor(sens))

    if CONF_TEMPERATURE_WAERMEQUELLE_EINGANG in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_WAERMEQUELLE_EINGANG])
        cg.add(var.set_temperature_waermequelle_eingang_sensor(sens))

    if CONF_TEMPERATURE_KAELTEKREIS in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_KAELTEKREIS])
        cg.add(var.set_temperature_kaeltekreis_sensor(sens))

    if CONF_TEMPERATURE_mischkreis1_VORLAUF in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_mischkreis1_VORLAUF])
        cg.add(var.set_temperature_mischkreis1_vorlauf_sensor(sens))

    if CONF_TEMPERATURE_mischkreis1_VORLAUF_SOLL in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_mischkreis1_VORLAUF_SOLL])
        cg.add(var.set_temperature_mischkreis1_vorlauf_soll_sensor(sens))

    if CONF_TEMPERATURE_raumstation in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_raumstation])
        cg.add(var.set_temperature_raumstation_sensor(sens))
