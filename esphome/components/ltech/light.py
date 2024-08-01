import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, remote_transmitter
from esphome.const import (
    CONF_BLUE,
    CONF_COLOR_INTERLOCK,
    CONF_GREEN,
    CONF_RED,
    CONF_OUTPUT_ID,
    CONF_WHITE,
    CONF_OUTPUT,
)

ltech_ns = cg.esphome_ns.namespace("ltech")
#LTECHLightOutput = ltech_ns.class_("LTECHLightOutput", light.LightOutput)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        #cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(LTECHLightOutput),
        cv.Required(CONF_OUTPUT): cv.use_id(remote_transmitter.RemoteTransmitterComponent),
        #cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
        #cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
        #cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
        #cv.Required(CONF_WHITE): cv.use_id(output.FloatOutput),
        #cv.Optional(CONF_COLOR_INTERLOCK, default=False): cv.boolean,
    }
)


async def to_code(config):
    #var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    #await light.register_light(var, config)

    transmitter = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_transmitter(transmitter))

