#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "TonyAP";
const char* password = "5252wifi";

WiFiUDP Udp;
unsigned int localUdpPort = 54321;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets


// FastLED
#include "FastLED.h"

#define LED_PINS    2
#define COLOR_ORDER GRB
#define CHIPSET     WS2811 // WS2811 LPD8806
#define NUM_LEDS    126

#define BRIGHTNESS  255  // reduce power consumption
#define LED_DITHER  255  // try 0 to disable flickering
#define CORRECTION  TypicalLEDStrip

int baseColorValue = 25;
// baseColorValue = 10.0;

CRGB leds[NUM_LEDS]; // Define the array of leds

int BRIGHTNESS_SCALE[] = {
    0, 1, 2, 3, 4, 5, 7, 9, 12, 15, 18, 22, 27, 32, 38, 44, 51, 58, 67, 76, 86, 96, 108, 120, 134,
    148, 163, 180, 197, 216, 235, 255
};
int MAX_BRIGHTNESS = sizeof(BRIGHTNESS_SCALE);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  //pinMode(2, OUTPUT);

  // FastLED setup
  FastLED.addLeds<CHIPSET, 2, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(CORRECTION);
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.setDither(LED_DITHER);
  FastLED.show(); // needed to reset leds to zero

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}


void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %i\n", incomingPacket);
    Serial.print(millis());

    int val = incomingPacket[0];
    // analogWrite(2, val);
    Serial.print("  ");
    Serial.println(val);

    //if ( val < baseColorValue) {
    //  val = baseColorValue;
    //}

    //CRGB color = CRGB::Blue;
    //color.nscale8_video(val);
    
    double now = millis() / 1000.0;
    double now_scaled = now/5; //(slow down cycling)
    //int base = max(0, baseColorValue-val);
    //One Tony Liked:
    //CRGB color = CRGB(pow(sin(now_scaled),2)* baseColorValue, val, pow(cos(now_scaled),2) * baseColorValue);
    CRGB color = CRGB(pow(sin(now_scaled),2)* baseColorValue, BRIGHTNESS_SCALE[min(val, MAX_BRIGHTNESS-1)], pow(cos(now_scaled),2) * baseColorValue);

    //CRGB color = CRGB(sin(now)* val, cos(now) * val, baseColorValue);
    //CRGB color = CRGB(pow(sin(now_scaled),2)* baseColorValue, min(255, int(pow(val, 1.5))), pow(cos(now_scaled),2) * baseColorValue);
    //double val_scaled = map(max(5,min(val,35)), 0, 35, 0, 255);
    //CRGB color = CRGB(abs(sin(now_scaled)* baseColorValue), val_scaled, abs(cos(now_scaled) * baseColorValue));
    
    /*double now = millis()/1000.0;
    double val_scaled = map(min(val,25), 0.0, 25.0, 4.0, 8.0);
    CHSV color = CHSV(int(now*255/10+val_scaled/2) % 255, 255, pow(2,val_scaled));
    Serial.printf("color: %d, %d, %d", color.hue, color.sat, color.val);
    */
    
    fill_solid(leds, NUM_LEDS, color);

    // Update Leds
    FastLED.show();
    //    // send back a reply, to the IP address and port we got the packet from
    //    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    //    Udp.write(replyPacket);
    //    Udp.endPacket();
  }
}
