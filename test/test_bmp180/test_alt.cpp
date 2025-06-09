// test_bmp180.cpp
#include <unity.h>
#include <Adafruit_BMP085.h>
#include "pins.h"

Adafruit_BMP085 bmp180;           // BMP085 driver also supports BMP180
#define SEALEVELPRESSURE_HPA 1013.25
#define BMP180_I2C_ADDR 0x77

void setupAltimeter() {
    if (!bmp180.begin(BMP180_I2C_ADDR))
        TEST_FAIL_MESSAGE("BMP180 not found – check wiring!");
}

void test_bmp180_basic_read() {
    for (uint8_t i = 0; i < 10; ++i) {
        int32_t pressure = bmp180.readPressure();            // Pa
        float altitude  = bmp180.readAltitude(SEALEVELPRESSURE_HPA);

        TEST_ASSERT_TRUE(pressure > 80000 && pressure < 110000);
        TEST_ASSERT_TRUE(altitude  > -500   && altitude  < 10000);

        delay(10);  // ~100 Hz
    }
}

void setup() {
    Serial.begin(115200);
    UNITY_BEGIN();
    setupAltimeter();
    RUN_TEST(test_bmp180_basic_read);
    UNITY_END();
}
void loop() {}
