#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define MASTER // Comment this define to compile sketch on slave device
const int8_t ledPin = LED_BUILTIN;
const char* const ssid = "TonyAP"; // Your network SSID (name)
const char* const pass = "5252wifi"; // Your network password
const uint16_t localPort = 54321; // Local port to listen for UDP packets
#ifdef MASTER
const uint32_t stepDuration = 500;
IPAddress broadcastAddress;
#endif

WiFiUDP udp;

bool sendPacket(const IPAddress& address, const uint8_t* buf, uint8_t bufSize);
void receivePacket();

//--MSGEQ7 //

#include "MSGEQ7.h"
#define pinAnalog A0
#define pinReset 15
#define pinStrobe 0
#define MSGEQ7_INTERVAL ReadsPerSecond(50)
#define MSGEQ7_SMOOTH 191 // Range: 0-255

CMSGEQ7<MSGEQ7_SMOOTH, pinReset, pinStrobe, pinAnalog> MSGEQ7;

// Choose a PWM pin!
#define pinLed 16


void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(ledPin, OUTPUT);
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  // This will set the IC ready for reading
  MSGEQ7.begin();
  // led setup
  pinMode(pinLed, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin, ! digitalRead(ledPin));
    delay(500);
    Serial.print('.');
  }
  digitalWrite(ledPin, HIGH);
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

#ifdef MASTER
  broadcastAddress = (uint32_t)WiFi.localIP() | ~((uint32_t)WiFi.subnetMask());
  Serial.print(F("Broadcast address: "));
  Serial.println(broadcastAddress);
#endif

  Serial.println(F("Starting UDP"));
  udp.begin(localPort);
  Serial.print(F("Local port: "));
  Serial.println(udp.localPort());
}

void loop() {


  if (udp.parsePacket())
    receivePacket();

  // Analyze without delay every interval
  bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);

  static uint32_t nextTime = 0;

  // Led output
  if (newReading) {
    // Read bass frequency
    uint8_t input = MSGEQ7.get(MSGEQ7_BASS);
    input = mapNoise(input);

    // Output PWM signal via Led to the music beat
    analogWrite(pinLed, input);
    Serial.print("Sound level: ");
    Serial.print(input);
    Serial.print("   ");
    Serial.println(millis());


    if (millis() >= nextTime) {
      bool led = ! digitalRead(ledPin);
      digitalWrite(ledPin, led);
      Serial.print(F("Turn led "));
      Serial.println(led ? F("off") : F("on"));
      if (! sendPacket(broadcastAddress, (uint8_t*)&input, sizeof(input)))
        Serial.println(F("Error sending broadcast UDP packet!"));
      nextTime = millis() + stepDuration;
    }

    delay(1);
  }
}

bool sendPacket(const IPAddress & address, const uint8_t* buf, uint8_t bufSize) {
  udp.beginPacket(address, localPort);
  udp.write(buf, bufSize);
  return (udp.endPacket() == 1);
}

void receivePacket() {
  bool led;

  udp.read((uint8_t*)&led, sizeof(led));
  udp.flush();
#ifdef MASTER
  if (udp.destinationIP() != broadcastAddress) {
    Serial.print(F("Client with IP "));
    Serial.print(udp.remoteIP());
    Serial.print(F(" turned led "));
    Serial.println(led ? F("off") : F("on"));
  } else {
    Serial.println(F("Skip broadcast packet"));
  }
#else
  digitalWrite(ledPin, led);
  led = digitalRead(ledPin);
  Serial.print(F("Turn led "));
  Serial.println(led ? F("off") : F("on"));
  if (! sendPacket(udp.remoteIP(), (uint8_t*)&led, sizeof(led)))
    Serial.println(F("Error sending answering UDP packet!"));
#endif
}
