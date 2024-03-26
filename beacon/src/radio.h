// LoRa
#include <SPI.h>
#include <RH_RF95.h>

// LoRa radio module pins for adafruit feather
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3 // 3 for feather m0, 7 for 32u4
#define RF95_FREQ 915.0 // radio frequency, must match RX's freq!

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void radio_init(void) {
  // manually reset radio module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.
  while (!rf95.init()) {
    Serial.println("ERROR: LoRa radio init failed!");
    //Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    digitalWrite(13, HIGH);
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("ERROR: setFrequency failed");
    //digitalWrite(13, HIGH);
    while (1);
  }

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}
