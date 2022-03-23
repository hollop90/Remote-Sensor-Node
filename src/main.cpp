/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses ABP (Activation-by-personalisation), where a DevAddr and
 * Session keys are preconfigured (unlike OTAA, where a DevEUI and
 * application key is configured, while the DevAddr and session keys are
 * assigned/generated in the over-the-air-activation procedure).
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!
 *
 * To use this sketch, first register your application and device with
 * the things network, to set or generate a DevAddr, NwkSKey and
 * AppSKey. Each device should have their own unique values for these
 * fields.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <LowPower.h>
#include <ClosedCube_HDC1080.h>
#include <Melopero_RV3028.h>

// Variables
Melopero_RV3028 rtc;
ClosedCube_HDC1080 hdc1080;
uint8_t LED_PIN = 8;
static uint8_t mydata[4];

// Function Declarations
void onEvent (ev_t ev);

void initFunc(osjob_t* j);
void wakeUp(osjob_t* j);
void readSensor(osjob_t* j);
void logData(osjob_t* j);
void do_send(osjob_t* j);
void sleep(osjob_t* j);

// Job structs
static osjob_t initJob;
static osjob_t wakeJob;
static osjob_t readJob;
static osjob_t logJob;
static osjob_t sendjob;
static osjob_t sleepJob;

void setupChannelsEU868();

// LoRaWAN NwkSKey, MSB first
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0xC8, 0x85, 0x1A, 0xB2, 0x03, 0x36, 0xD5, 0x01, 0x44, 0x5F, 0x79, 0xC3, 0x9C, 0xF2, 0x4E };

// LoRaWAN AppSKey, MSB first
static const u1_t PROGMEM APPSKEY[16] = { 0x59, 0x45, 0x55, 0xE0, 0xA6, 0x40, 0x3E, 0x59, 0x04, 0x58, 0x1D, 0x44, 0xF9, 0x83, 0xD1, 0x38 };

// LoRaWAN end-device address MSB first
static const u4_t DEVADDR = 0x260B540A ; // <-- Change this address for every node!

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 15;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,                       // chip select on feather (rf95module) CS
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,                       // reset pin
    .dio = {2, 6, LMIC_UNUSED_PIN}, 
};

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Wire.begin();
    Serial.begin(115200);
    delay(100);     // per sample code on RF_95 test
    Serial.println(F("Starting"));
    
    // //     rtc.initI2C();
    // //     rtc.set24HourMode();
    // //     rtc.setTime(2022, 3, 2, 21, 0, 30, 0);
    // // while (1)
    // // {
    // //     Serial.println(rtc.getUnixTime());
    // //     delay(100);
    // // }
    
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

    //  // os time accuracy
    // Serial.println(os_getTime());
    // Serial.flush();
    // LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
    // Serial.println(os_getTime());
    // Serial.flush();
    // while(1);
    initFunc(&initJob);
}

void loop() {
    os_runloop_once();
}




void initFunc(osjob_t* j){
    Serial.println("Init Job");
    Serial.flush();
    hdc1080.begin(0x40);
    rtc.initI2C();
    // Set the device to use the 24hour format (default) instead of the 12 hour format
  rtc.set24HourMode();

  // Set the date and time:
  rtc.setTime(2020, 9, 3, 30, 15, 20, 0);


    os_setCallback(&readJob, readSensor);
}

void wakeUp(osjob_t* j){
    Serial.println("Wake");
    Serial.flush();
}

void readSensor(osjob_t* j){
    Serial.println("Reading sensor");

    int senseVal = hdc1080.readHumidity() * 100;
    mydata[0] = highByte(senseVal);
    mydata[1] = lowByte(senseVal);
    
    senseVal = hdc1080.readTemperature() * 100;
    mydata[2] = highByte(senseVal);
    mydata[3] = lowByte(senseVal);
    os_setCallback(&logJob, logData);
}

void logData(osjob_t* j){
    Serial.println("Logging");
    Serial.flush();
    os_setCallback(&sendjob, do_send);
    ////os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
}

//The job pointer parameter is not used but it can be used if needed
void do_send(osjob_t* j){ // The job struct is passed to make sure that the cb calls the correct funtion for this job
    Serial.println("Sending");
    Serial.flush();
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Read sensors
        // Log Data 
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void sleep(osjob_t* j){
    // // unsigned long prevtime = millis();
    // // unsigned long nowtime = millis();
    // // for (int i = 0; i < 30; i++){
    // //     while (nowtime - prevtime < 1000UL){
    // //         os_runloop_once();
    // //         nowtime = millis();
    // //     }
    // //     prevtime = nowtime;
    // //     Serial.print("Delay: ");
    // //     Serial.println(i);
    // //     Serial.flush();        
    // // }
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    delay(10000);
    os_setCallback(&readJob, readSensor);
    ////os_setTimedCallback(&readJob, sec2osticks(TX_INTERVAL), readSensor);
}

void setupChannelsEU868(){
    
   LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
//    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        /*case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;            
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;*/
        case EV_TXCOMPLETE:
            //sleep emits tx start
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            
            // Schedule next transmission
            ////os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);  
            digitalWrite(LED_PIN, HIGH);
            delay(50);
            digitalWrite(LED_PIN, LOW);
            os_setCallback(&sleepJob, sleep);
            
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            //readsensor
            //transmitsata emits txcomplete
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}