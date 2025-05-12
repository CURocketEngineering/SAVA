#include <Arduino.h>

#ifdef SIM
  #include "simulation/Serial_Sim_LSM6DSOX.h"
  #include "simulation/Serial_Sim_LIS3MDL.h"
  #include "simulation/Serial_Sim_BMP390.h"
  #include "simulation/Serial_Sim.h"
#else
  #include "Adafruit_LSM6DSOX.h"
  #include "Adafruit_LIS3MDL.h"
  #include <Async_BMP3XX.h>
  #include "Adafruit_MPL3115A2.h"
#endif


#include <Adafruit_Sensor.h>
#include "pins.h"
#include "UARTCommandHandler.h"

#include "data_handling/SensorDataHandler.h"
#include "data_handling/DataSaver.h"
#include "data_handling/DataSaverBigSD.h"
#include "data_handling/DataNames.h"

#define SEALEVELPRESSURE_HPA (1013.25)


int last_led_toggle = 0;
int led_toggle_delay = 1000;
float loop_count = 0;
uint32_t start_time_s = 0;

DataSaverBigSD dataSaver(SD_CS);

Adafruit_BMP3XX     bmp180;
SensorDataHandler   bmp180_pressureData(CURE_BMP180_PRESSURE, &dataSaver);
SensorDataHandler   bmp180_altitudeData(CURE_BMP180_ALTITUDE, &dataSaver);

Adafruit_BMP3XX     bmp388;
SensorDataHandler   bmp388_pressureData(CURE_BMP388_PRESSURE, &dataSaver);
SensorDataHandler   bmp388_altitudeData(CURE_BMP388_ALTITUDE, &dataSaver);

Adafruit_MPL3115A2  mpl3115a2;
SensorDataHandler   mpl3115a2_pressureData(CURE_MPL3115A2_PRESSURE, &dataSaver);
SensorDataHandler   mpl3115a2_altitudeData(CURE_MPL3115A2_ALTITUDE, &dataSaver);

void setup() {


  Serial.begin(115200);
  // while (!Serial) delay(10); // Wait for Serial Monitor (Comment out if not using)

  Serial.println("Setting up BMP180...");
  while (!bmp180.begin_I2C(BMP180_ADDRESS)){
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    delay(10);
  }


  Serial.println("Setting up BMP388...");
  while (!bmp388.begin_I2C(BMP388_ADDRESS)){
    Serial.println("Could not find a valid BMP388 sensor, check wiring!");
    delay(10);
  }


  Serial.println("Setting up MPL3115A2...");
  while (!mpl3115a2.begin()){
    Serial.println("Could not find a valid MPL3115A2 sensor, check wiring!");
    delay(10);
  }

  mpl3115a2.setMode(MPL3115A2_BAROMETER);

  Serial.println("Setting up data saver...");

  // Initalize data saver
  if (!dataSaver.begin()) {
    Serial.println("Failed to initialize data saver");
  }

  Serial.println("Setup complete!");


  // Loop start time
  start_time_s = millis() / 1000;

  // Start first conversions
  bmp180.setConversionDelay(10);
  bmp388.setConversionDelay(10);
  bmp180.startConversion();
  bmp388.startConversion();
  mpl3115a2.startOneShot();
}

float pressureToAltitude(float pressure) {
  // Convert pressure to altitude using the barometric formula
  return 44330.0 * (1.0 - pow(pressure / 100.0f / SEALEVELPRESSURE_HPA, 0.1903));
}

void loop() {

  loop_count += 1;

  uint32_t current_time = millis();
  if (current_time - last_led_toggle > led_toggle_delay) {
    last_led_toggle = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }

  #ifdef SIM
  SerialSim::getInstance().update();
  #endif



  // Save data from all three altimeters
  if (bmp180.updateConversion()) {
    float pressure = bmp180.getPressure();
    float altitude = pressureToAltitude(pressure);
    bmp180_pressureData.addData(DataPoint(current_time, pressure));
    bmp180_altitudeData.addData(DataPoint(current_time, altitude));

    bmp180.startConversion();
  }

  if (bmp388.updateConversion()) {
    float pressure = bmp388.getPressure();
    float altitude = pressureToAltitude(pressure);
    bmp388_pressureData.addData(DataPoint(current_time, pressure));
    bmp388_altitudeData.addData(DataPoint(current_time, altitude));

    bmp388.startConversion();
  }

  if (mpl3115a2.conversionComplete()) {
    float pressure = mpl3115a2.getLastConversionResults(MPL3115A2_PRESSURE);
    float altitude = pressureToAltitude(pressure);
    mpl3115a2_pressureData.addData(DataPoint(current_time, pressure));
    mpl3115a2_altitudeData.addData(DataPoint(current_time, altitude));
    mpl3115a2.startOneShot();
  }

  // Throttle to 100 Hz
  int too_fast = millis() - current_time;  // current_time was captured at the start of the loop
  if (too_fast < 10) {
    delay(10 - too_fast);
  }
}