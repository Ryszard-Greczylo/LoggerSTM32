#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Pure logic: converts a raw ADC reading into a temperature in Celsius.
 * No hardware access, so it builds and is tested on the PC.
 *
 * Assumed circuit (a voltage divider):
 *
 *   Vref (3.3 V) ---[ R_ref ]---+---[ NTC ]--- GND
 *                               |
 *                          ADC input
 *
 * The ADC is 12-bit, so a reading is 0..4095 and represents Vadc / Vref.
 */

#define THERMISTOR_ADC_MAX_CODE 4096.0f /* 2^12, ideal full-scale step count */

typedef struct {
    float r_ref_ohm;  /* fixed resistor in the divider, e.g. 10000 */
    float r0_ohm;     /* NTC resistance at t0, e.g. 10000 at 25 C   */
    float t0_kelvin;  /* reference temperature, 298.15 K = 25 C     */
    float beta;       /* NTC beta constant, e.g. 3950               */
} thermistor_config_t;

/* Default for a 10k NTC, beta 3950, with a 10k reference resistor. */
extern const thermistor_config_t THERMISTOR_DEFAULT;

/*
 * Resistance of the NTC from a raw ADC code.
 * Returns false if the code is 0 or full scale (open or shorted sensor).
 */
bool thermistor_adc_to_ohm(const thermistor_config_t *cfg, uint16_t adc_code,
                           float *ohm_out);

/*
 * Temperature in Celsius from a raw ADC code, using the beta equation.
 * Returns false if the reading is invalid (sensor fault).
 */
bool thermistor_adc_to_celsius(const thermistor_config_t *cfg,
                               uint16_t adc_code, float *celsius_out);

#endif /* THERMISTOR_H */
