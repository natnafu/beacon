#include "compass.h"
#include "gps.h"
#include "pixels.h"
#include "radio.h"

typedef struct {
  uint8_t ID;
  float gps_lat;
  float gps_lng;
} Beacon;

#define SELF  0
#define OTHER 1

Beacon beacons[2] = {
  [SELF]  = {SELF , 0, 0},
  [OTHER] = {OTHER, 0, 0},
};

// Timer for sending a LORA transmission every TX_FREQ_MS
#define TX_FREQ_MS  5000
unsigned long tx_timer;

// Timer for outputing status info
#define STATUS_FREQ_MS  5000
unsigned long st_timer;

void setup(void) 
{
  Serial.begin(115200);
  Serial.println("Doing the Thing!");

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pixels_init();

  radio_init();
  tx_timer = millis();
  Serial.println("radio_init success!");

  gps_init();
  Serial.println("gps_init success!");

  // Runs calibration on each boot 
  compass_init();
  Serial.println("compass_init success!");

  pixels_set_all(0,0,0);
}

double Heading;
double path;

void loop()
{  
  // Status blinky
  digitalWrite(13, !digitalRead(13));
  
  // Get compass heading
  Heading = compass_get_heading();
  Serial.print("Heading: "); Serial.println(Heading);

  // Update GPS
  gps_update(); // needs to be called every loop
  beacons[SELF].gps_lat = gps_get_lat();
  beacons[SELF].gps_lng = gps_get_lng();

  if ((millis() - tx_timer > TX_FREQ_MS) &&
        beacons[SELF].gps_lat &&
        beacons[SELF].gps_lng) {
    // Transmit every 5s if lat and long are valid
    radio_tx_self();

    radio_rx_other();
    
    tx_timer = millis();
  }

  // calculate path to other beacon
  if (beacons[SELF].gps_lat == 0 || beacons[SELF].gps_lng == 0) {
    pixels_set_all(0,0,5);  // SELF location not valid
  } else if (beacons[OTHER].gps_lat == 0 || beacons[OTHER].gps_lng == 0) {
    pixels_set_all(0,5,0);  // OTHER location not valid
  } else {
    double courseToOther =
          TinyGPSPlus::courseTo(
            beacons[SELF].gps_lat,
            beacons[SELF].gps_lng,
            beacons[OTHER].gps_lat, 
            beacons[OTHER].gps_lng);
    
    // calculate which LED points towards other beacon
    path = courseToOther - Heading;
    if (path < 0.0) path += 360.0;
    if (path > 360.0) path -= 360.0;
  
    pixels_set_all(0,0,0);
    pixels_set_one((uint32_t(path*24.0/360.0)), 25,0,0);
  }

  // Status message
  if (millis() - st_timer > STATUS_FREQ_MS) {
    Serial.println("-GPS-");
    Serial.print("SELF: "); Serial.print(beacons[SELF].gps_lat,6);
    Serial.print(","); Serial.println(beacons[SELF].gps_lng,6);
    Serial.print("OTHER: "); Serial.print(beacons[OTHER].gps_lat,6);
    Serial.print(","); Serial.println(beacons[OTHER].gps_lng,6);
    Serial.println("-COMPASS-");
    Serial.print("Heading: "); Serial.println(Heading);
    Serial.print("Path: "); Serial.println(path);
    st_timer = millis();
  }
}

/////////////////////////// RADIO STUFF /////////////////////////////////////

// Sends data over LoRa
uint8_t radio_tx_self() {
  if (sizeof(Beacon) > RH_RF95_MAX_MESSAGE_LEN) {
    Serial.println("ERROR: size of TX Beacon array exceeds max msg len!");
    return 1;
  }
  delay(10);
  rf95.send((uint8_t*) &beacons[SELF], sizeof(Beacon));
  delay(10);
  rf95.waitPacketSent();
  Serial.println("TX success!");
  return 0;
}
 
uint8_t radio_rx_other() {
  if (rf95.waitAvailableTimeout(1)) {
    // Should be a reply message for us now
    Beacon *temp;
    uint8_t len;
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    if (rf95.recv(buf, &len))
    {
      temp = (Beacon *)buf;
      beacons[OTHER].ID = temp->ID;
      beacons[OTHER].gps_lat = temp->gps_lat;
      beacons[OTHER].gps_lng = temp->gps_lng;
      Serial.println("RX success!");
      return 0;
    }
    else
    {
      Serial.println("Receive failed");
      return 1;
    }
  }
  else
  {
    return 1;
  }
}
