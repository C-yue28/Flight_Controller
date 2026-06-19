#include <Arduino.h>
#include <Wire.h>

#include <BQ25798.h>
#include <OV5640.h>

// I2C pins

#define SDA 8
#define SCL 9

#define NUM_I2C_DEVICES 6

// External SD card

#define SD_CS 10
#define SD_MOSI 11
#define SD_SCK 12
#define SD_MISO 13

// Camera: OV5640

#define CAM_D2 1
#define CAM_D3 2
#define CAM_D4 3
#define CAM_D5 4
#define CAM_D6 5
#define CAM_D7 6
#define CAM_D8 7
#define CAM_D9 14
#define CAM_AVAIL_PC 38
#define CAM_FRAME_VS 39
#define CAM_ROW_HS 40

// IMU: LSM6DSOX
#define IMU_INT1 41
#define IMU_INT2 42

// I2C  

BQ25798 charger(SDA, SCL);

int detectI2CDevices();

void setup() {

  Serial.begin(115200);
  while (!Serial);

  Serial.print("Serial initialized\n");

  bool success_I2C = Wire.begin(SDA, SCL);
  if (success_I2C) {
    Serial.println("I2C initialized");
  } else {
    Serial.println("I2C initialization failed");
  }

  int numDevices = detectI2CDevices();
  while (numDevices < NUM_I2C_DEVICES) {
    Serial.printf("%d devices found\n", numDevices);
    Serial.println("Not enough I2C devices found, rechecking in 5 seconds");
    delay(5000);
    numDevices = detectI2CDevices();
  }
  Serial.printf("Successfully detected %d devices!\n", numDevices);

  // I2C device initialization

  // BQ25798

  if (charger.begin()) {
    Serial.println("BQ25798 initialized");
  } else {
    Serial.println("BQ25798 initialization failed");
  }

  if (charger.initialize3SDefaults()) {
    Serial.println("BQ25798 3S defaults initialized");
  } else {
    Serial.println("BQ25798 3S defaults initialization failed");
  }

  // OV5640

  if (initializeCamera(
    CAM_D2, CAM_D3, CAM_D4, CAM_D5,
    CAM_D6, CAM_D7, CAM_D8, CAM_D9,
    CAM_AVAIL_PC, CAM_FRAME_VS, CAM_ROW_HS,
    SDA, SCL,
    -1, -1
  )) {
    Serial.println("OV5640 initialized");
  } else {
    Serial.println("OV5640 initialization failed");
  }

}

void loop() {



}

int detectI2CDevices() {

  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning for devices...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(address, HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      Serial.println(address, HEX);
    }
  }

  return nDevices;

}