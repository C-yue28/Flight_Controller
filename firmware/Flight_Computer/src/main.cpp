#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>

// Personal libraries
#include <BQ25798.h>
#include <TCA9534.h>
#include <OV5640.h>
#include <MPL3115A2.h>

// Open-source libraries
#include <Adafruit_LSM6DSOX.h>

// I2C pins

#define SDA 8
#define SCL 9

#define NUM_I2C_DEVICES 5
#define MAX_I2C_INIT_LOOPS 10

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
#define IMU_I2C_ADDR 0b1101011

#define IMU_INT1 41
#define IMU_INT2 42

// MPL3115A2 (altimeter)
#define MPL3115A2_INT1 37
#define MPL3115A2_INT2 36

// I2C  

BQ25798 charger(&Wire);
TCA9534 motorController(&Wire);
MPL3115A2 altimeter(&Wire);
Adafruit_LSM6DSOX imu;
Adafruit_Sensor *imu_temp, *imu_gyro, *imu_accel;

sensors_vec_t rotation;
sensors_vec_t acceleration;
sensors_event_t imu_temp_event;

bool initI2C();
int detectI2CDevices();

bool writeFile(const char *path, const char *data, bool overwrite = false);
uint8_t* readFile(const char *path);

void readIMUData(uint8_t intNum);

void setup() {

  Serial.begin(115200);
  while (!Serial);

  Serial.print("Serial initialized\n");

  if (!initI2C()) {
    Serial.println("I2C initialization failed");
    return;
  }
  Serial.println("I2C initialized");

  // SD card

  if (!SD.begin(SD_CS)) {
    Serial.println("SD initialization failed");
    return;
  } else {
    Serial.println("SD initialized successfully");
  }

}

// Unit tests

/* OV5640

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
      Serial.println("Camera capture failed!");
      delay(1000);
      return;
  }

  Serial.printf("Captured image! Size: %d bytes, Width: %d, Height: %d\n", fb->len, fb->width, fb->height);

  esp_camera_fb_return(fb); 
  
  delay(100); 
}

*/

void loop() {

}

bool initI2C() {
  bool success_I2C = Wire.begin(SDA, SCL);
  if (success_I2C) {
    Serial.println("I2C initialized");
  } else {
    Serial.println("I2C initialization failed");
    return false;
  }

  int numDevices = detectI2CDevices();
  int loops = 0;
  while (numDevices < NUM_I2C_DEVICES) {
    Serial.printf("%d devices found\n", numDevices);
    Serial.println("Not enough I2C devices found, rechecking in 5 seconds");
    delay(5000);
    numDevices = detectI2CDevices();
    loops++;
    if (loops > MAX_I2C_INIT_LOOPS) {
      Serial.println("Max I2C init loops reached, stopping program");
      return false;
    }
  }
  Serial.printf("Successfully detected %d devices!\n", numDevices);

  // I2C device initialization

  // BQ25798

  if (charger.begin()) {
    Serial.println("BQ25798 initialized");
  } else {
    Serial.println("BQ25798 initialization failed");
    return false;
  }

  if (charger.initialize3SDefaults()) {
    Serial.println("BQ25798 3S defaults initialized");
  } else {
    Serial.println("BQ25798 3S defaults initialization failed");
    return false;
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
    return false;
  }

  // TCA9534
  motorController.begin();
  Serial.println("TCA9534 initialized");

  // MPL3115A2
  altimeter.begin();
  pinMode(MPL3115A2_INT1, OUTPUT_OPEN_DRAIN | INPUT);
  pinMode(MPL3115A2_INT2, OUTPUT_OPEN_DRAIN | INPUT);

  //attachInterrupt(digitalPinToInterrupt(MPL3115A2_INT1), do stuff when threshold maybe, FALLING);
  
  attachInterrupt(digitalPinToInterrupt(MPL3115A2_INT2), [&altimeter]() {
    altimeter.readAllData();
  }, LOW);
  
  Serial.println("MPL3115A2 altimeter initialized with interrupts");

  if (!imu.begin_I2C(IMU_I2C_ADDR)) {
    Serial.println("Failed to initialize IMU!");
    return false;
  }
  Serial.println("IMU initialized");

  imu.configIntOutputs(true, true);
  imu.configInt1(true, true, false, false, false);
  imu.configInt2(false, false, true);

  pinMode(IMU_INT1, OUTPUT_OPEN_DRAIN | INPUT);
  pinMode(IMU_INT2, OUTPUT_OPEN_DRAIN | INPUT);

  attachInterrupt(digitalPinToInterrupt(IMU_INT1), []() { readIMUData(0x00); }, LOW);
  attachInterrupt(digitalPinToInterrupt(IMU_INT2), []() { readIMUData(0x01); }, LOW);

  Serial.println("IMU interrupts configured");

  imu_temp = imu.getTemperatureSensor();
  imu_temp->printSensorDetails();

  imu_accel = imu.getAccelerometerSensor();
  imu_accel->printSensorDetails();

  imu_gyro = imu.getGyroSensor();
  imu_gyro->printSensorDetails();

  Serial.println("IMU sensors configured");

  return true;

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

bool writeFile(const char *path, const char *data, bool overwrite) {
  Serial.printf("Writing to file: %s\n", path);
  File file = SD.open(path, FILE_WRITE);
  if (file) {
    file.print(data);
    file.close();
    Serial.println("Write succeeded");
    return true;
  } else {
    Serial.println("Failed to write");
    return false;
  }
}


uint8_t* readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);
  File file = SD.open(path);
  size_t len = file.size();
  uint8_t* buffer = (uint8_t*)malloc(len+1);
  if (file) {
    file.read(buffer, len);
    buffer[len] = '\0';
    file.close();
  } else {
    Serial.println("Failed to read");
    return NULL;
  }
  return buffer;
}

void readIMUData(uint8_t intNum) {
  Serial.printf("IMU interrupt %d triggered\n", intNum);
  sensors_event_t accel;
  sensors_event_t gyro;
  imu_temp->getEvent(&imu_temp_event);
  imu_accel->getEvent(&accel);
  imu_gyro->getEvent(&gyro);

  rotation = gyro.gyro;
  acceleration = accel.acceleration;
  
  Serial.printf("Temperature: %.2f C\n", imu_temp_event.temperature);
  Serial.printf("Acceleration: %.2f m/s^2, %.2f m/s^2, %.2f m/s^2\n", acceleration.x, acceleration.y, acceleration.z);
  Serial.printf("Gyro: %.2f rad/s, %.2f rad/s, %.2f rad/s\n", rotation.x, rotation.y, rotation.z);
}

