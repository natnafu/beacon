#include "pixels.h"
#include "radio.h"

#define PIN_LED 13

uint8_t radio_tx_self();
uint8_t radio_rx_other();

typedef struct {
  uint8_t ID;
} Beacon;

#define SELF  0
#define OTHER 1

Beacon beacons[2] = {
  [SELF]  = {SELF},
  [OTHER] = {OTHER},
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
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pixels_init();

  radio_init();
  tx_timer = millis();
  Serial.println("radio_init success!");

  pixels_set_all(0,0,0);
}

void loop()
{  
  // Status blinky
  // digitalWrite(PIN_LED, !digitalRead(PIN_LED));


  if ((millis() - tx_timer > TX_FREQ_MS)) {
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    radio_tx_self();
    radio_rx_other();    
    tx_timer = millis();
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
