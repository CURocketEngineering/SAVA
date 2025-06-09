// test_mpl3115a2.cpp
#include <unity.h>
#include <Adafruit_MPL3115A2.h>
#include "pins.h"

Adafruit_MPL3115A2 mpl;

#define SEALEVELPRESSURE_HPA 1013.25

void setupAltimeter() {
    if (!mpl.begin())
        TEST_FAIL_MESSAGE("MPL3115A2 not found – check wiring!");

    mpl.setMode(MPL3115A2_BAROMETER);     // pressure mode
}

void test_mpl3115a2_basic_read() {
    for (uint8_t i = 0; i < 10; ++i) {
        float pressure = mpl.getLastConversionResults(MPL3115A2_PRESSURE);  // Pa
        float altitude = 44330.0f * (1.0f -
                         pow(pressure / 100.0f / SEALEVELPRESSURE_HPA,
                             0.1903f));

        TEST_ASSERT_TRUE(pressure > 80000 && pressure < 110000);
        TEST_ASSERT_TRUE(altitude > -500 && altitude < 10000);

        mpl.startOneShot();   // kick off next reading
        delay(10);            // ~100 Hz total cadence
        while (!mpl.conversionComplete()) {/* spin */}
    }
}

void setup() {
    Serial.begin(115200);
    UNITY_BEGIN();
    setupAltimeter();
    RUN_TEST(test_mpl3115a2_basic_read);
    UNITY_END();
}
void loop() {}
