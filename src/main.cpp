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

#include "FlashDriver.h"
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

DataSaverBigSD dataSaver();

Adafruit_BMP3XX     bmp180;
SensorDataHandler   bmp180_pressureData(BMP180_PRESSURE, &dataSaver);
SensorDataHandler   bmp180_altitudeData(BMP180_ALTITUDE, &dataSaver);

Adafruit_BMP3XX     bmp388;
SensorDataHandler   bmp388_pressureData(BMP388_PRESSURE, &dataSaver);
SensorDataHandler   bmp388_altitudeData(BMP388_ALTITUDE, &dataSaver);

Adafruit_MPL3115A2  mpl3115a2;
SensorDataHandler   mpl3115a2_pressureData(MPL3115A2_PRESSURE, &dataSaver);
SensorDataHandler   mpl3115a2_altitudeData(MPL3115A2_ALTITUDE, &dataSaver);

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

  Serial.println("Setting up data saver...");

  // Initalize data saver
  if (!dataSaver.begin()) {
    Serial.println("Failed to initialize data saver");
  }

  Serial.println("Setup complete!");


  // Loop start time
  start_time_s = millis() / 1000;

  // Simulation stuff

  #ifdef SIM
  while (!Serial) delay(10);
  SerialSim::getInstance().begin(&Serial, &stateMachine); 
  dataSaver.clearPostLaunchMode(); // Clear plm for sim
  #endif

}

void loop() {

  loop_count += 1;

  uint32_t current_time = millis();
  if (current_time - last_led_toggle > led_toggle_delay) {
    last_led_toggle = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }

  // Explicitly save a timestamp to ensure that all data from this loop is associated with the same timestamp and distinct from the previous loop
  dataSaver.saveTimestamp(current_time, TIMESTAMP);

  flightIDSaver.addData(DataPoint(current_time, flightID));

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  sensors_event_t mag_event; 

  // Cannot use cmdLine in SIM mode b/c they use the same
  // serial port
  #ifdef SIM
  SerialSim::getInstance().update();
  #else 
  cmdLine.readInput();
  #endif

  sox.getEvent(&accel, &gyro, &temp);

  DataPoint xAclDataPoint(current_time, accel.acceleration.x);
  DataPoint yAclDataPoint(current_time, accel.acceleration.y);
  DataPoint zAclDataPoint(current_time, accel.acceleration.z);

  xAclData.addData(xAclDataPoint);
  yAclData.addData(yAclDataPoint);
  zAclData.addData(zAclDataPoint);

  mag.getEvent(&mag_event);

  xMagData.addData(DataPoint(current_time, mag_event.magnetic.x));
  yMagData.addData(DataPoint(current_time, mag_event.magnetic.y));
  zMagData.addData(DataPoint(current_time, mag_event.magnetic.z));



  // Check periodically if a new reading is available
  if (bmp.updateConversion()) {
   
    float pres = bmp.getPressure();
    #ifdef SIM
      float alt = bmp.getAlt();
    #else
      // Simulation data might not store pressure in the same units, while meters is standard for alt
      float alt = 44330.0 * (1.0 - pow(pres / 100.0f / SEALEVELPRESSURE_HPA, 0.1903));
    #endif
    float temp = bmp.getTemperature();

    
    tempData.addData(DataPoint(current_time, temp));
    pressureData.addData(DataPoint(current_time, pres));
    altDataPoint.data = alt;
    altDataPoint.timestamp_ms = current_time;
    altitudeData.addData(altDataPoint);
    
    // Immediately start the next conversion
    bmp.startConversion();
  }

  xGyroData.addData(DataPoint(current_time, gyro.gyro.x));
  yGyroData.addData(DataPoint(current_time, gyro.gyro.y));
  zGyroData.addData(DataPoint(current_time, gyro.gyro.z));

  superLoopRate.addData(DataPoint(current_time, loop_count / (millis() / 1000 - start_time_s)));

  // Throttle to 100 Hz
  int too_fast = millis() - current_time;  // current_time was captured at the start of the loop
  if (too_fast < 10) {
    delay(10 - too_fast);
  }
}