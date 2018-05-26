#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "TonyAP";
const char* password = "5252wifi";

WiFiUDP Udp;
unsigned int localUdpPort = 54321;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets


// FastLED
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#define LED_PINS    2
#define COLOR_ORDER GRB
#define CHIPSET     WS2811 // WS2811 LPD8806
#define NUM_LEDS    126

#define BRIGHTNESS  255  // reduce power consumption
#define LED_DITHER  255  // try 0 to disable flickering
#define CORRECTION  TypicalLEDStrip

#define FRAMES_PER_SECOND  120

const int8_t ledPin = LED_BUILTIN;

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

  //Wifi setup
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(ledPin, ! digitalRead(ledPin));
    delay(500);
    Serial.print(".");
  }


  Serial.println(" connected");

  //Start UPD server
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{
  if (WiFi.status() == WL_CONNECTED ) {
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
      double now_scaled = now / 10; //(slow down cycling)
      //int base = max(0, baseColorValue-val);
      //One Tony Liked:
      //CRGB color = CRGB(pow(sin(now_scaled),2)* baseColorValue, val, pow(cos(now_scaled),2) * baseColorValue);
      CRGB color = CRGB(pow(sin(now_scaled), 2) * baseColorValue, BRIGHTNESS_SCALE[min(val, MAX_BRIGHTNESS - 1)], pow(cos(now_scaled), 2) * baseColorValue);

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
    }
  }
  else {
    gPatterns[gCurrentPatternNumber]();
  }
  // Update Leds
  FastLED.show();
  //    // send back a reply, to the IP address and port we got the packet from
  //    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  //    Udp.write(replyPacket);
  //    Udp.endPacket();

  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  EVERY_N_SECONDS( 10 ) {
    nextPattern();  // change patterns periodically
  }
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

