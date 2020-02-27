#pragma once

#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define NUM_PIXELS 24
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void pixels_set_one(int i, byte r, byte b, byte g) {
  pixels.setPixelColor(i, pixels.Color(r,g,b));
  pixels.show();
}

void pixels_set_all(byte r, byte b, byte g) {
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(r,g,b));
  }
  pixels.show();
}

void pixels_init() {
  pixels.begin();
  pixels_set_all(0,0,0);
  pixels.show();
}
