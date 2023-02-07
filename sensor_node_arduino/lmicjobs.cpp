/**
 * @file lmicjobs.cpp
 * @author Ugochukwu Uzoukwu
 * @brief Every job to be scheduled by the LMIC
 * @version 0.9
 * @date 2023-02-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "lmicJobs.h"
#include "SensorNodeDefs.h"
#include <Arduino.h>

#include <lmic.h>
#include <RV3028C7.h>
#include <ClosedCube_HDC1080.h>
#include <LowPower.h>

RV3028C7 rtc;
ClosedCube_HDC1080 hdc;
uint8_t mydata[6];

osjob_t initJob;
osjob_t wakeJob;
osjob_t readJob;
osjob_t logJob;
osjob_t sendjob;
osjob_t sleepJob;

void initFunc(osjob_t* j){
    // Blink on startup
    for (int i = 0; i < 3; i++) {
        delay(500);
        digitalWrite(8, HIGH);
        delay(100);
        digitalWrite(8, LOW);
        delay(100);
    }
    Serial.println("Init Job");
    Serial.flush();

    Wire.begin();
    rtc.begin();
    hdc.begin(0x40);

    os_setCallback(&readJob, readSensor);
}

void wakeUp(osjob_t* j){
    Serial.println("Wake");
    Serial.flush();
}

void readSensor(osjob_t* j){
    Serial.println("Reading sensor");
    Serial.flush();

    // Compacting data for transmission
    int senseVal = hdc.readHumidity() * 100;
    mydata[0] = highByte(senseVal);
    mydata[1] = lowByte(senseVal);
    
    senseVal = hdc.readTemperature() * 100;
    mydata[2] = highByte(senseVal);
    mydata[3] = lowByte(senseVal);

    int battVolt = readBattVoltage() * 100;
    Serial.println(battVolt);
    mydata[4] = highByte(battVolt);
    mydata[5] = lowByte(battVolt);
    os_setCallback(&logJob, logData);
}

void logData(osjob_t* j){
    // SD Card not yet implemented
    Serial.println("Logging");
    Serial.flush();
    os_setCallback(&sendjob, do_send);
    ////os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
}

//I'm still not sure what the job argument is for
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
    // Enable sleep timer interrupt and go to sleep
    
    attachInterrupt(digitalPinToInterrupt(3), wakeUp, FALLING);
    Serial.println("Going to sleep...");
    Serial.flush();
    rtc.setPeriodicCountdownTimer(WAKE_INTERVAL, TIMER_1HZ);
    rtc.enableInterrupt(INTERRUPT_PERIODIC_COUNTDOWN_TIMER);
    rtc.startPeriodicCountdownTimer();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    Serial.println("Waking up!!!");
    Serial.flush();
    rtc.stopPeriodicCountdownTimer();
    rtc.clearInterrupt(INTERRUPT_PERIODIC_COUNTDOWN_TIMER);
    rtc.disableInterrupt(INTERRUPT_PERIODIC_COUNTDOWN_TIMER);
    detachInterrupt(digitalPinToInterrupt(3));
    
    os_setCallback(&readJob, readSensor);
}

double readBattVoltage(){
    digitalWrite(BATT_CTRL, HIGH);
    delay(5);
    int adcReading = analogRead(A2);
    delay(5);
    digitalWrite(BATT_CTRL, LOW);
    //char buff[50];
    double rescaledVolt = (adcReading * 0.003222656)/0.8; // Magic values are ADC Volt/LSB (@3.3V) and voltage diveder factor
    //sprintf(buff, "ADC: %d \t Volt: %f", adcReading, rescaledVolt);
    Serial.println(rescaledVolt);
    return rescaledVolt;
}

void wakeUp(){}

void setupChannelsEU868(){    
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    #ifndef INDOOR_TESTING
        LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
        LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    #endif

    #ifdef INDOOR_TESTING
        LMIC_disableChannel(1);
        LMIC_disableChannel(2);
        LMIC_disableChannel(3);
        LMIC_disableChannel(4);
        LMIC_disableChannel(5);
        LMIC_disableChannel(6);
        LMIC_disableChannel(7);
        LMIC_disableChannel(8); 
    #endif
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
            digitalWrite(LED_G, HIGH);
            delay(200);
            digitalWrite(LED_G, LOW);
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
