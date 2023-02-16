#include "esp_camera.h"
#include <WiFi.h>
#include "Arduino.h"
// Time
#include "time.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"
// MicroSD
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory

// define the number of bytes you want to access
#define EEPROM_SIZE 1

// Edit ssid, password, capture_interval:
const char* ssid = "Homers_House";
const char* password = "Veigarlover420";
int capture_interval = 5000; // milliseconds between captures
//

long current_millis;
long last_capture_millis = 0;
static esp_err_t cam_err;
static esp_err_t card_err;
char strftime_buf[64];
bool internet_connected = false;
struct tm timeinfo;
time_t now;

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();

  if (init_wifi()) { // Connected to WiFi
    internet_connected = true;
    Serial.println("Internet connected");
    init_time();
    time(&now);
    setenv("TZ", "GMT0BST,M3.5.0/01,M10.5.0/02", 1);
    tzset();
  }

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
}

bool init_wifi()
{
  int connAttempts = 0;
  Serial.println("\r\nConnecting to: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
    if (connAttempts > 10) return false;
    connAttempts++;
  }
  return true;
}

void init_time()
{
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();
  // wait for time to be set
  time_t now = 0;
  timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 10;
  while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    Serial.printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
    delay(2000);
    time(&now);
    localtime_r(&now, &timeinfo);
  }
}

static void save_photo()
{
  camera_fb_t * fb = NULL;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb); 
}

void loop()
{
  current_millis = millis();
  if (current_millis - last_capture_millis > capture_interval) { // Take another picture
    last_capture_millis = millis();
    save_photo();
  }
}