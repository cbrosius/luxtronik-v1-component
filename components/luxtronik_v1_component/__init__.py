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
CONF_TEMPERATURE_MISCHKREIS1_VORLAUF = "temperature_mischkreis1_vorlauf"
CONF_TEMPERATURE_MISCHKREIS1_VORLAUF_SOLL = "temperature_mischkreis1_vorlauf_soll"
CONF_TEMPERATURE_RAUMSTATION = "temperature_raumstation"
CONF_EINGANG_ABTAU_SOLEDRUCK_DURCHFLUSS = "eingang_abtau_soledruck_durchfluss"
CONF_EINGANG_SPERRZEIT_EVU = "eingang_sperrzeit_evu"
CONF_EINGANG_HOCHDRUCKPRESSOSTAT = "eingang_hochdruckpressostat"
CONF_EINGANG_MOTORSCHUTZ = "eingang_motorschutz"
CONF_EINGANG_NIEDERDRUCKPRESSOSTAT = "eingang_niederdruckpressostat"
CONF_EINGANG_FREMDSTROMANODE = "eingang_fremdstromanode"
CONF_AUSGANG_ABTAUVENTIL = "ausgang_abtauventil"
CONF_AUSGANG_BWP = "ausgang_bwp"
CONF_AUSGANG_FBHP = "ausgang_fbhp"
CONF_AUSGANG_HZP = "ausgang_hzp"
CONF_AUSGANG_MISCHER_1_AUF = "ausgang_mischer_1_auf"
CONF_AUSGANG_MISCHER_1_ZU = "ausgang_mischer_1_zu"
CONF_AUSGANG_VENT_WP = "ausgang_vent_wp"
CONF_AUSGANG_VENT_BRUNNEN = "ausgang_vent_brunnen"
CONF_AUSGANG_VERDICHTER_1 = "ausgang_verdichter_1"
CONF_AUSGANG_VERDICHTER_2 = "ausgang_verdichter_2"
CONF_AUSGANG_ZPUMPE = "ausgang_zpumpe"
CONF_AUSGANG_ZWE = "ausgang_zwe"
CONF_AUSGANG_ZWE_STOERUNG = "ausgang_zwe_stoerung"

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

INPUT_OUTPUT_SCHEMA = sensor.sensor_schema(
    state_class=STATE_CLASS_MEASUREMENT,
    unit_of_measurement="", # no unit
    accuracy_decimals=0,    # no decimals
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
        cv.Optional(CONF_TEMPERATURE_MISCHKREIS1_VORLAUF): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_MISCHKREIS1_VORLAUF_SOLL): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_TEMPERATURE_RAUMSTATION): TEMPERATURE_SCHEMA,
        cv.Optional(CONF_EINGANG_ABTAU_SOLEDRUCK_DURCHFLUSS): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_EINGANG_SPERRZEIT_EVU): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_EINGANG_HOCHDRUCKPRESSOSTAT): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_EINGANG_MOTORSCHUTZ): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_EINGANG_NIEDERDRUCKPRESSOSTAT): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_EINGANG_FREMDSTROMANODE): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_ABTAUVENTIL): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_BWP): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_FBHP): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_HZP): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_MISCHER_1_AUF): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_MISCHER_1_ZU): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_VENT_WP): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_VENT_BRUNNEN): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_VERDICHTER_1): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_VERDICHTER_2): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_ZPUMPE): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_ZWE): INPUT_OUTPUT_SCHEMA,
        cv.Optional(CONF_AUSGANG_ZWE_STOERUNG): INPUT_OUTPUT_SCHEMA,
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

    if CONF_TEMPERATURE_MISCHKREIS1_VORLAUF in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_MISCHKREIS1_VORLAUF])
        cg.add(var.set_temperature_mischkreis1_vorlauf_sensor(sens))

    if CONF_TEMPERATURE_MISCHKREIS1_VORLAUF_SOLL in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_MISCHKREIS1_VORLAUF_SOLL])
        cg.add(var.set_temperature_mischkreis1_vorlauf_soll_sensor(sens))

    if CONF_TEMPERATURE_RAUMSTATION in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_RAUMSTATION])
        cg.add(var.set_temperature_raumstation_sensor(sens))

    if CONF_EINGANG_ABTAU_SOLEDRUCK_DURCHFLUSS in config:
        sens = await sensor.new_sensor(config[CONF_EINGANG_ABTAU_SOLEDRUCK_DURCHFLUSS])
        cg.add(var.set_eingang_abtau_soledruck_durchfluss_sensor(sens))

    if CONF_EINGANG_SPERRZEIT_EVU in config:
        sens = await sensor.new_sensor(config[CONF_EINGANG_SPERRZEIT_EVU])
        cg.add(var.set_eingang_sperrzeit_evu_sensor(sens))

    if CONF_EINGANG_HOCHDRUCKPRESSOSTAT in config:
        sens = await sensor.new_sensor(config[CONF_EINGANG_HOCHDRUCKPRESSOSTAT])
        cg.add(var.set_eingang_hochdruckpressostat_sensor(sens))

    if CONF_EINGANG_MOTORSCHUTZ in config:
        sens = await sensor.new_sensor(config[CONF_EINGANG_MOTORSCHUTZ])
        cg.add(var.set_eingang_motorschutz_sensor(sens))

    if CONF_EINGANG_NIEDERDRUCKPRESSOSTAT in config:
        sens = await sensor.new_sensor(config[CONF_EINGANG_NIEDERDRUCKPRESSOSTAT])
        cg.add(var.set_eingang_niederdruckpressostat_sensor(sens))

    if CONF_EINGANG_FREMDSTROMANODE in config:
        sens = await sensor.new_sensor(config[CONF_EINGANG_FREMDSTROMANODE])
        cg.add(var.set_eingang_fremdstromanode_sensor(sens))
    if CONF_AUSGANG_ABTAUVENTIL in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_ABTAUVENTIL])
        cg.add(var.set_ausgang_abtauventil_sensor(sens))
    
    if CONF_AUSGANG_BWP in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_BWP])
        cg.add(var.set_ausgang_bwp_sensor(sens))
        
    if CONF_AUSGANG_FBHP in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_FBHP])
        cg.add(var.set_ausgang_fbhp_sensor(sens))
        
    if CONF_AUSGANG_HZP in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_HZP])
        cg.add(var.set_ausgang_hzp_sensor(sens))
        
    if CONF_AUSGANG_MISCHER_1_AUF in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_MISCHER_1_AUF])
        cg.add(var.set_ausgang_mischer_1_auf_sensor(sens))
        
    if CONF_AUSGANG_MISCHER_1_ZU in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_MISCHER_1_ZU])
        cg.add(var.set_ausgang_mischer_1_zu_sensor(sens))
        
    if CONF_AUSGANG_VENT_WP in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_VENT_WP])
        cg.add(var.set_ausgang_vent_wp_sensor(sens))
        
    if CONF_AUSGANG_VENT_BRUNNEN in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_VENT_BRUNNEN])
        cg.add(var.set_ausgang_vent_brunnen_sensor(sens))
        
    if CONF_AUSGANG_VERDICHTER_1 in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_VERDICHTER_1])
        cg.add(var.set_ausgang_verdichter_1_sensor(sens))
        
    if CONF_AUSGANG_VERDICHTER_2 in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_VERDICHTER_2])
        cg.add(var.set_ausgang_verdichter_2_sensor(sens))
        
    if CONF_AUSGANG_ZPUMPE in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_ZPUMPE])
        cg.add(var.set_ausgang_zpumpe_sensor(sens))
        
    if CONF_AUSGANG_ZWE in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_ZWE])
        cg.add(var.set_ausgang_zwe_sensor(sens))
        
    if CONF_AUSGANG_ZWE_STOERUNG in config:
        sens = await sensor.new_sensor(config[CONF_AUSGANG_ZWE_STOERUNG])
        cg.add(var.set_ausgang_zwe_stoerung_sensor(sens))