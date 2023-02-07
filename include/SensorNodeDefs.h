/**
 * @file SensorNodeDefs.h
 * @author Ugochukwu Uzoukwu
 * @brief Common definitions accross the projct
 * @version 1.0
 * @date 2023-02-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef UGO_NODE_DEF
#define UGO_NODE_DEF

// Pin definitions
#define LORA_SS     10
#define LORA_RST    17
#define LORA_PIN0   14
#define LORA_PIN1   5

#define LED_R       7
#define LED_G       8
#define LED_B       9

#define BATT_SENSE  16
#define BATT_CTRL   6

#define RTC_INT     3

// Other
#define WAKE_INTERVAL   600  // Unit: Seconds

//! Uncommonet to transmit on only 868.1 MHz THIS VIOLATES LORAWAN/TTN REGULATIONS
#define INDOOR_TESTING

#endif // UGO_NODE_DEF