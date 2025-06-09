// test_bmp388.cpp
#include <unity.h>
#include <Adafruit_BMP3XX.h>      // same driver family as BMP390
#include "pins.h"

Adafruit_BMP3XX bmp388;

#define SEALEVELPRESSURE_HPA 1013.25
#define BMP388_I2C_ADDR 0x76      // change if your board uses 0x76

void setupAltimeter() {
    // Initialise over I²C
    if (!bmp388.begin_I2C(BMP388_I2C_ADDR)) {
        TEST_FAIL_MESSAGE("BMP388 not found – check wiring / address!");
    }
    bmp388.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp388.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp388.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
}

void test_bmp388_basic_read() {
    for (uint8_t i = 0; i < 10; ++i) {
        if (!bmp388.performReading())
            TEST_FAIL_MESSAGE("BMP388 read failed");

        float pressure = bmp388.pressure;                               // Pa
        float altitude = 44330.0f * (1.0f -
                         pow(pressure / 100.0f / SEALEVELPRESSURE_HPA,
                             0.1903f));

        TEST_ASSERT_TRUE(pressure > 80000 && pressure < 110000);
        TEST_ASSERT_TRUE(altitude > -500 && altitude < 10'000);

        delay(10);  // ~100 Hz
    }
}

void setup() {
    Serial.begin(115200);
    UNITY_BEGIN();
    setupAltimeter();
    RUN_TEST(test_bmp388_basic_read);
    UNITY_END();
}
void loop() {}
