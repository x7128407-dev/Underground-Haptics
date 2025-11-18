/**
Written by Abul Al Arabi arabiabulal@gmail.com
The code comes as is without any warranty
data = {
  "mode": 1,
  "intensity": 0
}
**/

// Library files
// #include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
#include <ArduinoOTA.h>
// #include <esp_now.h>
// // file system
// #include <SPIFFS.h>
// #include <FS.h>
// #include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <esp_now.h>

#define TRIGGER_PIN 3

#define FILENAME "config.json"

// OTA wifi params
const char* ssid = "Netgear2.4";
const char* password = "projectHaptics";

// MAC address of the transmitter dongle
static uint8_t PEER[]{ 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x65 };
uint8_t TENS[] = { 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x69 };
esp_now_peer_info_t peerInfo;

// Structure for incoming data
typedef struct _message {
  int tMode;
  int tIntensity;
} _message;

_message inData;

// Mode and Intensity Settings
int cMode = 1;
int cIntensity = 0;
bool onUpdate = false;
#define EEPROMaddress 0

// Pulse length
#define pulseLen 50

// Pin Settings
#define modePin 5
#define intPin 10
#define otaPin 19

void setModeInt(int tMode, int tIntensity) {
  int dMode = tMode - cMode;
  int dIntensity = tIntensity - cIntensity;
  onUpdate = true;
  bool addDelay = false;
  // set the mode first as it resets intensity
  while (abs(dMode)) {
    addDelay = true;
    // as the intensity gets reset so set the diff to target
    dIntensity = tIntensity;
    pinMode(modePin, OUTPUT);
    if (dMode < 0) {
      digitalWrite(modePin, 0);
      dMode++;
    } else if (dMode > 0) {
      digitalWrite(modePin, 1);
      dMode--;
    }
    // delay for the pulse
    delay(pulseLen);

    // set the pins back to high impedence mode
    pinMode(modePin, INPUT);
    // delay for the pulse
    delay(pulseLen);
  }

  if (addDelay) delay(pulseLen);
  // now set intensity
  while (abs(dIntensity)) {
    pinMode(intPin, OUTPUT);
    if (dIntensity < 0) {
      digitalWrite(intPin, 0);
      dIntensity++;
    } else if (dIntensity > 0) {
      digitalWrite(intPin, 1);
      dIntensity--;
    }
    // delay for the pulse
    delay(pulseLen);

    // set the pins back to high impedence mode
    pinMode(intPin, INPUT);
    // delay for the pulse
    delay(pulseLen);
  }
  // update complete
  cMode = tMode;
  cIntensity = tIntensity;

  // save the mode to spiffs

  onUpdate = false;
}


// function to set pins
void setPins() {
  // pinMode(otaPin, INPUT_PULLUP);
  // digitalWrite(otaPin,HIGH);
  //pinMode(TRIGGER_PIN, OUTPUT);
  //digitalWrite(TRIGGER_PIN, HIGH);
  pinMode(modePin, INPUT);
  pinMode(intPin, INPUT);
}
  
void espNowSetup() {
  Serial.println("Initializing ESPNOW");
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    return;
  }
  Serial.println("WiFi initialized");


  Serial.println("Setting MAC");
  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, TENS);
  if (err == ESP_OK) {
    Serial.println("Successfully set MAC");
  } else {
    Serial.println("Failed to set MAC");
  }

  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
  Serial.println();

  // Once ESPNow is successfully Init, we will register for Send CB to
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, PEER, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    return;
  }
}

void setup() {
  Serial.begin(115200);
  // set the pins
  setPins();

  // check for OTA mode
  delay(500);
  if (digitalRead(otaPin) == 0) {
    otaMode();
  }

  // read the config file
  // if (SPIFFS.begin()) {
  //   File configFile = SPIFFS.open(FILENAME, "r");
  //   if (configFile) {
  //     size_t size = configFile.size();
  //     std::unique_ptr<char[]> buf(new char[size]);
  //     configFile.readBytes(buf.get(), size);
  //     DynamicJsonDocument doc(1024);
  //     DeserializationError error = deserializeJson(doc, buf.get());
  //     if (!error) {
  //       cMode = doc["mode"];
  //     }
  //   }
  //   else {
  //     // set the default mode
  //     cMode = 1;
  //   }

  cMode = 1;
  //}

  // if OTA was not triggered then go into espNow
  espNowSetup();
}

void loop() {
  // if this is empty then it can cause WDT reboot. so set something here.
  String arabi = "rocks";
}

// espNow handler function
void OnDataRecv(const esp_now_recv_info_t* mac, const uint8_t* incomingData, int len) {
  // digitalWrite(TRIGGER_PIN, LOW);
  memcpy(&inData, incomingData, sizeof(inData));
  if ((inData.tMode > 0 && inData.tMode < 31) && (inData.tIntensity >= 0 && inData.tIntensity <= 20)) {
    if (!onUpdate) setModeInt(inData.tMode, inData.tIntensity);
  }
  // digitalWrite(TRIGGER_PIN, HIGH);
  // // save the mode to spiffs
  // DynamicJsonDocument doc(1024);
  // doc["mode"] = cMode;
  // File configFile = SPIFFS.open(FILENAME, "w");
  // if (configFile) {
  //   serializeJson(doc, configFile);
  //   configFile.close();
  // }
}

// OTA handler function
void otaMode() {
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // additional settings
  // ArduinoOTA.setPort(3232);
  // ArduinoOTA.setHostname("myesp32");
  ArduinoOTA.setPassword("admin");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  while (1) {
    ArduinoOTA.handle();
  }
}
