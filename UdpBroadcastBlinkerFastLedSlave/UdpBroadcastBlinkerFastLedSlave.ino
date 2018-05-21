#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int8_t statusLed = LED_BUILTIN;
const int8_t ledBlink = 2;

const char* const ssid = "TonyAP"; // Your network SSID (name)
const char* const pass = "5252wifi"; // Your network password

const uint16_t localPort = 54321; // Local port to listen for UDP packets


WiFiUDP udp;

bool sendPacket(const IPAddress& address, const uint8_t* buf, uint8_t bufSize);
void receivePacket();

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(statusLed, OUTPUT);
  pinMode(ledBlink, OUTPUT);

  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(statusLed, ! digitalRead(statusLed));
    delay(500);
    Serial.print('.');
  }
  digitalWrite(statusLed, HIGH);
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  Serial.println(F("gateway ip is"));
  Serial.println(WiFi.gatewayIP());

  Serial.println(F("subnet is "));
  Serial.println(WiFi.subnetMask());

  Serial.println(F("Starting UDP"));
  udp.begin(localPort);
  Serial.print(F("Local port: "));
  Serial.println(udp.localPort());
}

void loop() {


  if (udp.parsePacket())
    receivePacket();

  delay(1);
}
//
//bool sendPacket(const IPAddress& address, const uint8_t* buf, uint8_t bufSize) {
//  udp.beginPacket(address, localPort);
//  udp.write(buf, bufSize);
//  return (udp.endPacket() == 1);
//}

void receivePacket() {
  // bool led;
  int ledVal;

  udp.read((uint8_t*)&ledVal, sizeof(ledVal));
  udp.flush();
  analogWrite(ledBlink, ledVal);
  //digitalWrite(ledPin, led);
  //led = digitalRead(ledPin);
  Serial.print("Received value: ");
  Serial.println(ledVal);
  //  if (! sendPacket(udp.remoteIP(), (uint8_t*)&ledVal, sizeof(ledVal)))
  //    Serial.println(F("Error sending answering UDP packet!"));

}
