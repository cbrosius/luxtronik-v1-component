import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    CONF_ID,
)
from . import CONF_LUXTRONIK_V1_ID, luxtronik_v1_ns, LuxtronikV1Component

DEPENDENCIES = ['luxtronik_v1']

LuxtronikV1Sensor = luxtronik_v1_ns.class_('LuxtronikV1Sensor', sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend({
        cv.GenerateID(): cv.declare_id(LuxtronikV1Component),
        cv.Required(CONF_LUXTRONIK_V1_ID): cv.use_id(LuxtronikV1Component),
    })
)

async def to_code(config):
    """Generate code for Luxtronik V1 temperature sensor."""
    # Get the parent component (controller) first
    controller = await cg.get_variable(config[CONF_LUXTRONIK_V1_ID])
    
    # Create and register the sensor
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    # Register with controller AFTER sensor is fully initialized
    cg.add(controller.register_sensor(var))