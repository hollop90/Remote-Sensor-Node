/**
 * @file main.cpp
 * @author Ugochukwu Uzoukwu
 * @brief A LoRaWAN temperature and humidity sensor node. My final year project
 * @version 1.0
 * @date 2023-02-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include "SensorNodeDefs.h"
#include "lmicJobs.h"

// LoRaWAN NwkSKey, MSB first
static const u1_t NWKSKEY[16] = {};

// LoRaWAN AppSKey, MSB first
static const u1_t APPSKEY[16] = {};

// LoRaWAN end-device address MSB first
static const u4_t DEVADDR = 0x0 ; // <-- Change this address for every node!

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
    LMIC_setSession (0x13, DEVADDR, NWKSKEY, APPSKEY);
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
