#include "thermistor.h"

#include <math.h>
#include <stddef.h>

const thermistor_config_t THERMISTOR_DEFAULT = {
    .r_ref_ohm = 10000.0f,
    .r0_ohm = 10000.0f,
    .t0_kelvin = 298.15f,
    .beta = 3950.0f,
};

/*
 * Divider maths (NTC on the bottom, as drawn in thermistor.h):
 *
 *   Vadc / Vref = R_ntc / (R_ref + R_ntc)
 *   code / 4096 = R_ntc / (R_ref + R_ntc)
 *
 * Solving for R_ntc:
 *
 *   R_ntc = R_ref * code / (4096 - code)
 *
 * Vref cancels out, so the result does not depend on the 3.3 V value as long
 * as the ADC and the divider use the same reference.
 */
bool thermistor_adc_to_ohm(const thermistor_config_t *cfg, uint16_t adc_code,
                           float *ohm_out)
{
    if (cfg == NULL || ohm_out == NULL) {
        return false;
    }
    /* 0 means shorted to ground, 4095 and above means open circuit. */
    if (adc_code == 0 || adc_code >= (uint16_t)(THERMISTOR_ADC_MAX_CODE - 1.0f)) {
        return false;
    }

    const float code = (float)adc_code;
    *ohm_out = cfg->r_ref_ohm * code / (THERMISTOR_ADC_MAX_CODE - code);
    return true;
}

/*
 * Beta equation:
 *
 *   1/T = 1/T0 + (1/B) * ln(R / R0)       (T in kelvin)
 *
 * It is a good approximation near T0 and drifts slightly at the extremes.
 * Steinhart-Hart is more accurate over a wide range and can replace this
 * function later without changing the interface.
 */
bool thermistor_adc_to_celsius(const thermistor_config_t *cfg,
                               uint16_t adc_code, float *celsius_out)
{
    float r_ntc;

    if (celsius_out == NULL || !thermistor_adc_to_ohm(cfg, adc_code, &r_ntc)) {
        return false;
    }

    const float inv_t = (1.0f / cfg->t0_kelvin) +
                        (logf(r_ntc / cfg->r0_ohm) / cfg->beta);

    *celsius_out = (1.0f / inv_t) - 273.15f;
    return true;
}
