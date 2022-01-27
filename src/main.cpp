// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

//DHT11 DEFINES
#define DHTTYPE DHT22
#define DHTPIN 5

//RF95 DEFINES
#define CS_PIN 4
#define INT_PIN 3

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// DHT Sensor instance
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;

// Singleton instance of the radio driver
RH_RF95 driver(CS_PIN, INT_PIN);
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init()){
    Serial.println("init failed");
  }
  dht.begin();
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    driver.setFrequency(868.3);
    driver.setTxPower(14, false);
    //driver.setSpreadingFactor(7);

  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);
}

uint8_t data[1];
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  Serial.println("Sampling DHT");
  dht.humidity().getEvent(&event);
  dht.temperature().getEvent(&event);
  data [0] = event.temperature;
  //Serial.println()
 
  //data [1] = event.relative_humidity;
  Serial.println("Sending to rf95_reliable_datagram_server");
    
  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
  delay(500);
}

