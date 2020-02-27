#pragma once

#include <TinyGPS++.h>

// GPS module settings
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;

void gps_init(void) {
  Serial1.begin(GPSBaud);
}

// Gives TinyGPS++ object NMEA data from GPS module
void gps_update(void) {
  while (Serial1.available()){ 
    gps.encode(Serial1.read());
 }
}

// Returns latitude if locaiton is valid, else return 0
float gps_get_lat(void) {
  if (gps.location.isValid()) {
    return gps.location.lat();
  } else {
    return 0;
  }
}

// Returns longitude if locaiton is valid, else return 0
float gps_get_lng(void) {
  if (gps.location.isValid()) {
    return gps.location.lng();
  } else {
    return 0;
  }
}
