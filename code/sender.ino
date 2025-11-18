/**
Written by Abul Al Arabi arabiabulal@gmail.com
The code comes as is without any warranty
**/

#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <esp_now.h>

#define PREC 6

uint8_t gateWayMac[] = { 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x65 };
static uint8_t nodes[2][6] = { { 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x66 }, { 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x67 } };

uint8_t VIBRO[] = { 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x68 };
uint8_t TENS[] = { 0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x69 }; //0x34, 0x85, 0x18, 0x20, 0xFD, 0x18

esp_now_peer_info_t peerInfo;

// Structure for incoming data
typedef struct _sensor_message {
  char msg;
} _sensor_message;

_sensor_message sensor_msg;

// Structure for incoming sensor data
typedef struct _imudata {
  float gx, gy, gz, ax, ay, az, mx, my, mz, roll, pitch, heading;
} _imudata;
_imudata nodeData;

//two int for tens/ems
// Structure for incoming data
typedef struct _tens_msg {
  int tMode;
  int tIntensity;
} _tens_msg;

_tens_msg tens_msg;

// three int for vibro
// Structure for incoming data
typedef struct _vibro_msg {
  int id;
  int pwmValue;
  int pulseDuration;
  int pulseSpacing;
} _vibro_msg;

_vibro_msg vibro_msg;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Booting...");
  initWireless();
}

void loop() {
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '1') triggerNode(0);
    else if (ch == '2') triggerNode(1);
    else if (ch == 'T') {
      // TENS Module Data
      String tensData;
      while (ch != '\n') {
        ch = Serial.read();
        if (ch == '\n') break;
        tensData = tensData + String(ch);
      }
      tensData.trim();
      int a = tensData.indexOf(',');
      // if the separator exists
      if (a != -1) {
        // cut off the data parts
        String m = tensData.substring(0, a);
        String intensity = tensData.substring(a + 1);

        // convert to integers
        tens_msg.tMode = m.toInt();
        tens_msg.tIntensity = intensity.toInt();

        sendTens();
      }
    } else if (ch == 'V') {
      // VIBRO Module Data
      String vibroData;
      while (ch != '\n') {
        ch = Serial.read();
        if (ch == '\n') break;
        vibroData += String(ch);
      }
      vibroData.trim();
      // Parse the VIBRO data, assuming it comes in the format: id,pwmValue,pulseDuration,pulseSpacing
      int idIndex = vibroData.indexOf(',');
      int pwmIndex = vibroData.indexOf(',', idIndex + 1);
      int durationIndex = vibroData.indexOf(',', pwmIndex + 1);

      if (idIndex != -1 && pwmIndex != -1 && durationIndex != -1) {
        String id = vibroData.substring(0, idIndex);
        String pwmValue = vibroData.substring(idIndex + 1, pwmIndex);
        String pulseDuration = vibroData.substring(pwmIndex + 1, durationIndex);
        String pulseSpacing = vibroData.substring(durationIndex + 1);

        // Convert to integers
        vibro_msg.id = id.toInt();
        vibro_msg.pwmValue = pwmValue.toInt();
        vibro_msg.pulseDuration = pulseDuration.toInt();
        vibro_msg.pulseSpacing = pulseSpacing.toInt();
        
        // Serial.print(vibro_msg.id);
        // Serial.print(',');
        // Serial.print(vibro_msg.pwmValue);
        // Serial.print(',');
        // Serial.print(vibro_msg.pulseDuration);
        // Serial.print(',');
        // Serial.println(vibro_msg.pulseSpacing);
        sendVibro();
      }
    }
  }
}
