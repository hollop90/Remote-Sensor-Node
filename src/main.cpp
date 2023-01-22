#include "Arduino.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SensorNodeDefs.h>
#include <lmicJobs.h>

// Uncommonet to transmit on only 868.1 MHz THIS VIOLATES LORAWAN/TTN REGULATIONS
#define INDOOR_TESTING

// LoRaWAN NwkSKey, MSB first
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0xC8, 0x85, 0x1A, 0xB2, 0x03, 0x36, 0xD5, 0x01, 0x44, 0x5F, 0x79, 0xC3, 0x9C, 0xF2, 0x4E };

// LoRaWAN AppSKey, MSB first
static const u1_t PROGMEM APPSKEY[16] = { 0x59, 0x45, 0x55, 0xE0, 0xA6, 0x40, 0x3E, 0x59, 0x04, 0x58, 0x1D, 0x44, 0xF9, 0x83, 0xD1, 0x38 };

// LoRaWAN end-device address MSB first
static const u4_t DEVADDR = 0x260B540A ; // <-- Change this address for every node!

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = LORA_SS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_PIN0, LORA_PIN1, LMIC_UNUSED_PIN},
};

void setup() {
    CLKPR = 0x80; // (1000 0000) enable change in clock frequency
    CLKPR = 0x01; // (0000 0001) use clock division factor 2 to reduce the frequency from 16 MHz to 8 MHz
    delay(100);     // per sample code on RF_95 test

    Serial.begin(115200);
    Serial.println(F("Starting Sensor Node"));
    pinMode(LED_G, OUTPUT);
    pinMode(BATT_CTRL, OUTPUT);

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x13, DEVADDR, NWKSKEY, APPSKEY);
    #endif
    setupChannelsEU868();

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink
    LMIC_setDrTxpow(DR_SF7,20);
    
    // Program flow starts here. Defined in **TBD**
    initFunc(&initJob);
}

void loop() {
    os_runloop_once();
}
