#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "esp_camera.h"
#include "ArduinoJson.h"
#include "Firebase_ESP_Client.h"
#include "SD_MMC.h"

// Replace with your network credentials
const char* ssid = "COMHEM_a78a76";
const char* password = "m2y2yd5c";

// Replace with your Firebase project details
String FIREBASE_HOST = "thesis-e256f.firebaseapp.com";
String API_KEY = "AIzaSyDfriP7PiwSikZGi5SAaeNK3BOL3s_u9ws";
String storageBucket = "your-firebase-project.appspot.com";

// Replace with your Firebase service account credentials
const char serviceAccount[] PROGMEM = R"rawliteral(
  {
    "type": "service_account",
    "project_id": "thesis-e256f",
    "private_key_id": "9b933ee51cd1c166c0e1d2ff9de2170c0becc4af",
    "client_email": "firebase-adminsdk-q07bn@thesis-e256f.iam.gserviceaccount.com",
    "client_id": "111465776282438721485",
    "auth_uri": "https://accounts.google.com/o/oauth2/auth",
    "token_uri": "https://oauth2.googleapis.com/token",
    "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
    "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-q07bn%40thesis-e256f.iam.gserviceaccount.com"
  }
)rawliteral";

const char* privateKey = R"KEY(
-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCv3PySji9IDezz\n1ppWMI49F1FI2zErVfoqUqggTRe8Av7IQlg0WwMebW5vZytTzg1ng0vihTY3VRPn\n2KHxmPUTFvGX41PHPWqQxvi87Oqgdls3JmE6KTv6ZHzAlsIfV/r3IlZa4mfo8H8Q\nY8sNfBcq6xj0TOuWmKIrTkE3z6YuAeEnXx0aQ4s/TEW3elFmdgKl6nTAHD6B4hSv\nPLMs9ZiUB22/NJTUEd8M+u9tKo4/46qhBBOKSztjfqlez3NdsVIZRw8HBABAz32g\nZfuga0DL8RGbYbvll1CIIcSGilK57kYdfT2znJUUb+Sv10asrarPcb5e8Lr/Xe36\nDC/fPt0VAgMBAAECggEABL3lTthwd0JdIVvbBs2BsNf/LK25HhKt7VAT5pBlCwjc\nTpm8apJOcxalVvpMObUL7IuEitGSSiC23RYlTyeK6oKV3IEYPu1LieJ3zfBF74eJ\n+HwskZ0HArYEPxlil6HCumusmKq+H5Ky4MYe/gEnefaBWvHBEo070RbWrCKz+/g1\nRtnq+xcapyBQ7CLVvmENJdCk532pu6XCNleeVuRVbCETFUTW2SGrOMwlqENEgwSi\n39uuQQyGCosbGcGL3pksX1IQgf8pqGMYuyqV8k714eHCzqrbc7ZKohYrNvxLSGFD\neDbVRaE4GDT6vQJONuTc37yfpWkfl1K+lOdMo653iQKBgQDghG/4aY3KuotdjLhH\ng1MA9ozmAstcsblbqOR3xmk8k91qSYxv3rlLy4CLFb/Bus/P61maY6Sft8LT0f2D\nkWEYX8FghwKBTfYXwPfo+g1ql9wtbgZqitZLqgBEo9E1xKMl4P9pz0gcUMZJEMQ9\n82b3Nj6Q9Wzi8kGUB+S+66JGVwKBgQDIhf+9uR6ehuls9adcXb0QuQ3F9v1fuHk8\nA4yJ4uf9SIdPUr55+VjLYSvOE/I/wjO97xIvmrnm2aMaU6V8a3H3PlElibsoNL52\no9JvH4DeSnaISPhD8ZgYdSpU7sBHfeWj4DZB94RqkpkNUbwgVBN9Is4qM2AoHGFJ\nNgKrzT5ccwKBgEbTQIc6tRIY2tH+DDvy/4I0RKpig0iQgAqcMsZd/uCvPI/E99pq\nhkWUws4dZEp25guPEMsVV99SltE/UxvoBpsrydLrhCzKLTG9G/eQuh7AHSLvrHJ7\nrbJcxIziqKKvj0/ptGjqjLLyADsYYO+yEu81W3NbGKhXB7zs6IoZxy0tAoGAeadS\nEHhO6z5OP5yJfLAiFBMxF/lQntH8Ozyr9ruCo1dtP+KjTLzJ8hlxygSv456eZYy1\nK8xjHHrNSRuWOnW1HprgADYfJbVaGAArmt8x/6qNXwFmHVJlQ/FRTVXLsZPfzhwo\nzryryTxHPp1n1PUtHa4QBepFXBBbtV7fH/vU6aECgYBRS+jz0/sKs5YCfhl3KYYq\nJP7OM1XCwXsUaqlW5sVFhXjo13yvyx9RMZtJQ75pmFPOS33TlVTAkCGWY2aShhdj\nCa/voaIODWB1FhO2bjNp4Yw3YNpGsIAhW7+fVVGeSIkgnZlna6v3RPYZloOnCkMx\n+vtbU3zYzEQVhsqSjxTBxg==\n-----END PRIVATE KEY-----\n
)KEY";

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