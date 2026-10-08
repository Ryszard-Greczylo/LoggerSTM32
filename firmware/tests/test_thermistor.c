/*
 * Minimal unit tests for thermistor.c, built and run on the PC.
 * No framework yet: a tiny CHECK macro is enough to start. Unity or
 * GoogleTest can replace it later.
 */
#include <math.h>
#include <stdio.h>

#include "thermistor.h"

static int failures = 0;

#define CHECK(cond, msg)                                           \
    do {                                                           \
        if (!(cond)) {                                             \
            printf("FAIL: %s (line %d)\n", (msg), __LINE__);       \
            failures++;                                            \
        }                                                          \
    } while (0)

static int near(float a, float b, float tol)
{
    return fabsf(a - b) <= tol;
}

/* Mid-scale reading: NTC equals the reference resistor, so exactly 25 C. */
static void test_midscale_is_25c(void)
{
    float t;
    CHECK(thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 2048, &t), "valid reading");
    CHECK(near(t, 25.0f, 0.01f), "2048 counts should be 25 C");
}

/* Resistance at mid-scale must equal the reference resistor. */
static void test_midscale_resistance(void)
{
    float r;
    CHECK(thermistor_adc_to_ohm(&THERMISTOR_DEFAULT, 2048, &r), "valid reading");
    CHECK(near(r, 10000.0f, 1.0f), "2048 counts should be 10 kOhm");
}

/* Hotter means lower NTC resistance, so a lower ADC code. */
static void test_monotonic(void)
{
    float t_low = 0.0f, t_mid = 0.0f, t_high = 0.0f;
    CHECK(thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 1000, &t_low), "low code");
    CHECK(thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 2048, &t_mid), "mid code");
    CHECK(thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 3000, &t_high), "high code");
    CHECK(t_low > t_mid && t_mid > t_high, "temperature must fall as the code rises");
}

/*
 * Round trip: pick a temperature, compute the NTC resistance it would have,
 * work out the ADC code the divider would give, convert back, compare.
 * Allowed error is about one ADC step near 25 C.
 */
static void test_round_trip(void)
{
    const float temps_c[] = {-20.0f, 0.0f, 25.0f, 50.0f, 80.0f};
    const thermistor_config_t *cfg = &THERMISTOR_DEFAULT;

    for (unsigned i = 0; i < sizeof(temps_c) / sizeof(temps_c[0]); i++) {
        const float t_k = temps_c[i] + 273.15f;
        const float r_ntc =
            cfg->r0_ohm * expf(cfg->beta * (1.0f / t_k - 1.0f / cfg->t0_kelvin));
        const float code_f = THERMISTOR_ADC_MAX_CODE * r_ntc / (cfg->r_ref_ohm + r_ntc);
        const uint16_t code = (uint16_t)lroundf(code_f);

        float t_back;
        CHECK(thermistor_adc_to_celsius(cfg, code, &t_back), "round trip valid");
        CHECK(near(t_back, temps_c[i], 0.6f), "round trip within 0.6 C");
    }
}

/* Open and shorted sensors must be reported as faults, not as numbers. */
static void test_faults(void)
{
    float t;
    CHECK(!thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 0, &t), "0 = shorted");
    CHECK(!thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 4095, &t), "4095 = open");
    CHECK(!thermistor_adc_to_celsius(NULL, 2048, &t), "null config");
    CHECK(!thermistor_adc_to_celsius(&THERMISTOR_DEFAULT, 2048, NULL), "null output");
}

int main(void)
{
    test_midscale_is_25c();
    test_midscale_resistance();
    test_monotonic();
    test_round_trip();
    test_faults();

    if (failures == 0) {
        printf("All thermistor tests passed.\n");
        return 0;
    }
    printf("%d check(s) failed.\n", failures);
    return 1;
}
