"""Python bindings for Genalyzer"""

# Version of the genalyzer bindings (which may be different than the library version)
__version__ = "0.0.1"
__author__ = "Analog Devices, Inc."


from .simplified import (
    config_free,
    config_gen_ramp,
    config_gen_tone,
    config_quantize,
    config_histz_nla,
    config_fftz,
    config_fa,
    gn_config_fa_auto,
    gen_ramp,
    gen_real_tone,
    gen_complex_tone,
    quantize,
    fftz,
    histz,
    get_ha_results,
    get_wfa_results,
    get_fa_single_result,
    get_fa_results,
    config_set_sample_rate,
    config_code_format,
)
import genalyzer.advanced as advanced
import genalyzer.helpers as helpers
