// 2015-02-03 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

/*
    Compile time configuration option
 */

#ifndef THS_CONFIG
#define THS_CONFIG

// define pin number where the DHT data pin is connected to
#define DHT_PIN       2
// define DHT type (see DHT library for supported types)
#define DHTTYPE       DHT22

// define pin number where the tx module data pin is connected to
#define TRANSMITTER_PIN 10

// define pin number for the transmission period indication led
//  - to disable indication led undef this
//#define TX_INDICATOR_LED_PIN 13

// minimum battery voltage in mV
//  - battery low indicator will be set in transmitted messages
//    if measured battery voltage is below this value
#define MIN_BATTERY_VOLTAGE      3500L

// default tx interval in minutes
//  - will be used for first time EEPROM initialization
#define DEFAULT_TX_INTERVAL  15

// number of message transmittion repetitions 
#define NUM_TX_REPETITIONS  4

#endif

