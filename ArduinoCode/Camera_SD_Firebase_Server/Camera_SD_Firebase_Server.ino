#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "esp_camera.h"
#include "ArduinoJson.h"
#include "Firebase_ESP_Client.h"
#include "SD_MMC.h"

// CHECK DATA.TXT for this info!!!

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseStorage storage;
FirebaseJson json;
WebServer server(80);
String serverIndex;

void startCameraServer();
void takePicture();
void savePictureToSDCard();
void sendPictureToFbStorage();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  config.signer.service_account.private_key = privateKey;

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Initialize SD card
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Initialize Firebase
  config.host = FIREBASE_HOST.c_str();
  config.api_key = API_KEY.c_str();
  config.signer.tokens.legacy_token = "";
  config.signer.tokens.id_token = "";
  config.signer.tokens.access_token = "";
  config.signer.tokens.refresh_token = "";
  config.signer.tokens.token_type = "";
  config.signer.tokens.expires_in = 0;
  config.signer.tokens.authenticated = false;
  config.signer.service_account.data = serviceAccount;
  config.signer.service_account.cert = nullptr;
  config.signer.service_account.project_id = "";
  config.signer.service_account.private_key = "";
  config.signer.service_account.email = "";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize Firebase Storage
  storage.begin(&config, &auth);

  // Set IP address in Firebase
  Firebase.setString(&fbdo, "esp32-cam-ip", WiFi.localIP().toString().c_str());

  // Start camera server
  startCameraServer();

  // Setup a timer to take pictures every 30 minutes
  const long interval = 30 * 60 * 1000;
  static esp_err_t take_picture_task(void *arg) {
    while (1) {
      takePicture();
      vTaskDelay(interval / portTICK_PERIOD_MS);
    }
  }
  xTaskCreatePinnedToCore(take_picture_task, "take_picture_task", 4096, NULL, 1, NULL, 0);
}

void loop() {
  // Handle HTTP requests
  server.handleClient();
}

void startCameraServer() {
  server.on("/", HTTP_GET, {
    server.send_P(200, "text/html", serverIndex);
  });
  server.on("/capture", HTTP_GET, {
    takePicture();
  });
  server.onNotFound( {
    server.send(404, "text/plain", "404: Not found");
  });
  server.begin();
  Serial.println("Camera server started");
}

void takePicture() {
  Serial.println("Taking picture...");

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
  Serial.println("Camera capture failed");
  return;
  }

  // Save the picture to the SD card
  savePictureToSDCard(fb);

  // Send the picture to the Firebase storage
  sendPictureToFbStorage(fb);

  // Return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);
}

void savePictureToSDCard(camera_fb_t * fb) {
  String path = "/picture_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if (!file) {
  Serial.println("Failed to open file for writing");
  return;
  }
  file.write(fb->buf, fb->len);
  file.close();
  Serial.println("Picture saved: " + path);
}

void sendPictureToFbStorage(camera_fb_t * fb) {
  Serial.println("Sending picture to Firebase...");
  String fileName = String("picture_") + String(millis()) + ".jpg";

  storage.setBucket(storageBucket.c_str());
  fbdo.setResponseSize(4096);
  fbdo.setWriteLimit(4096);
  storage.upload(&fbdo, fileName.c_str(), fb->buf, fb->len, "image/jpeg");

  if (fbdo.httpCode() == 200) {
   Serial.println("Picture uploaded: " + fileName);
  } else {
    Serial.println("Upload failed: " + fbdo.errorReason());
  }
}