#include "esp_camera.h"

camera_config_t config;

bool initializeCamera(
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
    uint8_t pclk, uint8_t vsync, uint8_t href,
    uint8_t sccb_sda, uint8_t sccb_scl,
    uint8_t pwdn, uint8_t reset
) {

    // DVP Bus Pin Mapping
    config.pin_d0 = d0;
    config.pin_d1 = d1;
    config.pin_d2 = d2;
    config.pin_d3 = d3;
    config.pin_d4 = d4;
    config.pin_d5 = d5;
    config.pin_d6 = d6;
    config.pin_d7 = d7;
    config.pin_pclk = pclk;
    config.pin_vsync = vsync;
    config.pin_href = href;
    config.pin_sccb_sda = sccb_sda;
    config.pin_sccb_scl = sccb_scl;
    config.pin_pwdn = -1;
    config.pin_reset = -1;

    // CRITICAL SETTINGS FOR ONBOARD XCLK:
    config.pin_xclk = -1;             // -1 disables the ESP32's PWM clock output generation
    config.xclk_freq_hz = 24000000;   // Informs the driver that the camera runs at 24MHz

    // Memory Allocation
    config.fb_count = 2;              // Use 2 for smoother frame-grabbing if PSRAM is active
    config.fb_location = CAMERA_FB_IN_PSRAM; 
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // Image format options
    config.pixel_format = PIXFORMAT_JPEG; 
    config.frame_size = FRAMESIZE_SVGA; // OV5640 supports up to QSXGA, but UXGA/SVGA is standard
    config.jpeg_quality = 48;           // 0-63, lower=higher quality

    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }
    return true;
}