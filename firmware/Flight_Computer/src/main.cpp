#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>

// Personal libraries
#include <BQ25798.h>
#include <TCA9534.h>
#include <OV5640.h>
#include <BME280.h>

// Open-source libraries
#include <Adafruit_LSM6DSOX.h>

#define ESP32_ADDR 0x67

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

// Motor handling
#define SERVO_1 0
#define SERVO_2 1
#define SERVO_3 2
#define MOTOR_1 4
#define MOTOR_2 5
#define MOTOR_3 6

#define PWM_FREQUENCY 1 // 1 tick, or 1 ms per bit on the output lines to ESC

// I2C  

BQ25798 charger(&Wire);
TCA9534 motorController(&Wire);
BME280 bme280(&Wire);
Adafruit_LSM6DSOX imu;
Adafruit_Sensor *imu_temp, *imu_gyro, *imu_accel;

sensors_vec_t rotation;
sensors_vec_t acceleration;
sensors_event_t imu_temp_event;

TaskHandle_t pwmHandle = NULL;

typedef struct {
  uint8_t pin;
  float pwm_duty;
} motorControlConfig;

bool initI2C();
int detectI2CDevices();

bool writeFile(const char *path, const char *data, bool overwrite = false);
uint8_t* readFile(const char *path);

void readIMUData(uint8_t intNum);
void outputPWM(void *pvParameters);

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

  // run the motors at 25% speed
  motorControlConfig* initial_pwm_config = new motorControlConfig();
  initial_pwm_config->pwm_duty = 0.25;
  for (int i = MOTOR_1; i < MOTOR_3; i++) {
    initial_pwm_config->pin = i;
    xTaskCreatePinnedToCore(outputPWM, "PWM", 4096, (void*)initial_pwm_config, 1, NULL, 0);
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

/* Motors

void loop() {
  motorControlConfig* cfg = new motorControlConfig();
  cfg->pwm_duty = 0.5;
  for (int i = MOTOR_1; i < MOTOR_3; i++) {
    cfg->pin = i;
    xTaskCreatePinnedToCore(outputPWM, "PWM", 4096, cfg, 1, NULL, 0);
  }
  
  cfg->pwm_duty = 0.0;
  for (int i = MOTOR_1; i < MOTOR_3; i++) {
    cfg->pin = i;
    xTaskCreatePinnedToCore(outputPWM, "PWM", 4096, cfg, 1, NULL, 0);
  }
}

*/

/* BME280

void loop() {
  Serial.printf("Temperature: %f C\n", bme280.getTemperature());
  Serial.printf("Pressure: %f Pa\n", bme280.getPressure());
  Serial.printf("Humidity: %f %%\n", bme280.getHumidity());
  vTaskDelay(2000 / portTICK_PERIOD_MS);
}

*/

/* IMU

void loop() {
  Serial.printf("Temperature: %f C\n", altimeter.getTemperature() + imu_temp_event.temperature);
  Serial.printf("Rotation: %f rad/s, %f rad/s, %f rad/s\n", rotation.x, rotation.y, rotation.z);
  Serial.printf("Acceleration: %f m/s^2, %f m/s^2, %f m/s^2\n", acceleration.x, acceleration.y, acceleration.z);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
}

*/

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

  // BME280
  bme280.begin();
  Serial.println("BME280 initialized");

  // IMU
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
  Serial.printf("IMU interrupt %d triggered\n", intNum+1);
  if (intNum == 0x00) {
    sensors_event_t gyro;
    imu_temp->getEvent(&imu_temp_event);
    imu_gyro->getEvent(&gyro);

    rotation = gyro.gyro;

    Serial.printf("Temperature: %.2f C\n", imu_temp_event.temperature);
    Serial.printf("Gyro: %.2f rad/s, %.2f rad/s, %.2f rad/s\n", rotation.x, rotation.y, rotation.z);
  } else {
    sensors_event_t accel;
    imu_accel->getEvent(&accel);

    acceleration = accel.acceleration;

    Serial.printf("Acceleration: %.2f m/s^2, %.2f m/s^2, %.2f m/s^2\n", acceleration.x, acceleration.y, acceleration.z);
  }
}

void outputPWM(void* pvParameters) {
  motorControlConfig* temp_config = (motorControlConfig*)pvParameters;
  motorControlConfig config;
  config.pin = temp_config->pin;
  config.pwm_duty = temp_config->pwm_duty;
  // Send 8-bit PWM after sending arbitrary 8-bit address - change to 4-bit later?
 
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (int i = 0; i < 8; i++) {
    motorController.digitalWrite(config.pin, (ESP32_ADDR >> i) & 0x01);
    vTaskDelayUntil(&xLastWakeTime, PWM_FREQUENCY);
  }
  
  uint8_t pwm_bits = (uint8_t)(config.pwm_duty * 255);
  xLastWakeTime = xTaskGetTickCount();

  for (int i = 0; i < 8; i++) {
    motorController.digitalWrite(config.pin, (pwm_bits >> i) & 0x01);
    vTaskDelayUntil(&xLastWakeTime, PWM_FREQUENCY);
  }

  motorController.digitalWrite(config.pin, LOW);
}

