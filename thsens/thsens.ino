// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

/*
 *  wireless tempereature/humidity sensor
 *
 */

//#define THS_DEBUG

#include <DHT.h>
#include <Wire.h>
#include <LowPower.h>
#include <TrueRandom.h>
#include <EEPROM.h>
#include <avr/power.h>

#include "thspreferences.h"
#include "thsutils.h"

// configuration section
// define pin number where the DHT data pin is connected to
#define DHT_PIN       2
// define DHT type
#define DHTTYPE       DHT22

// define pin number where the tx module data pin is connected to
#define TRANSMITTER_PIN 10

// define this to indicate transmission period via led
//#define TX_INDICATOR_LED_PIN 13

// minimum VCC in mV
//  (battery low indicator will be set in transmitted messages if measured voltage is below this value)
#define MIN_VCC      3500L

// end of configuration section

THSPreferences prefs;
THSUtils utils;
DHT dht(DHT_PIN, DHTTYPE);
uint8_t last = 0;
// kind of a hack (timer0_millis is the arduino core libs internal millis counter variable)
extern long timer0_millis;

void setup() {  
#ifdef THS_DEBUG
  // initialize serial:
  Serial.begin(57600);
  Serial.setTimeout(10);
#else  
  power_usart0_disable();
#endif

  // disable unused uC parts
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
  power_spi_disable();

  // setting unused IO pins to input/low saves some current too  
  for (byte pinnr = 0; pinnr <= A5; pinnr++) {
    pinMode(pinnr, INPUT); 
    digitalWrite(pinnr, LOW);
  }

  // init transmitter pin
  pinMode(TRANSMITTER_PIN, OUTPUT); 
  digitalWrite(TRANSMITTER_PIN, LOW);

  // init tx indicator led
#ifdef TX_INDICATOR_LED_PIN
  pinMode(TX_INDICATOR_LED_PIN, OUTPUT); 
  digitalWrite(TX_INDICATOR_LED_PIN, LOW);
#endif

  // temperature/humidity sensor dht22
  dht.begin();

  prefs.load();
  
  last = 0;
  sendEnv();
}

const int ENVrepetition = 4;

static void transmitBit(int nHighPulses, int nLowPulses) {
  digitalWrite(TRANSMITTER_PIN, HIGH);
  delayMicroseconds(nHighPulses);
  digitalWrite(TRANSMITTER_PIN, LOW);
  delayMicroseconds(nLowPulses);
}

#define SYNC_HI  200
#define SYNC_LO  8050
#define ZERO_HI 200
#define ZERO_LO 1010
#define ONE_HI  200
#define ONE_LO  2020
static void env_sendMessage(const char* message) {
#ifdef TX_INDICATOR_LED_PIN
  digitalWrite(TX_INDICATOR_LED_PIN, HIGH);
#endif

  for (int i = 0; i < ENVrepetition; i++) {
    unsigned int pos = 0;
    transmitBit(SYNC_HI,SYNC_LO);
    while (message[pos] != '\0') {
      switch(message[pos]) {
      case '0':
        transmitBit(ZERO_HI,ZERO_LO);
        break;
      case '1':
        transmitBit(ONE_HI,ONE_LO);
        break;
      }
      pos++;
    }
    switch(message[pos-1]) {
    case '0':
      transmitBit(ZERO_HI,ZERO_LO);
      break;
    case '1':
      transmitBit(ONE_HI,ONE_LO);
      break;
    }
    transmitBit(SYNC_LO,SYNC_LO);
  }

#ifdef TX_INDICATOR_LED_PIN
  digitalWrite(TX_INDICATOR_LED_PIN, LOW);
#endif
}

static String generateEnvMessage(uint8_t deviceId, uint8_t channel, bool battOk, bool manualSend, uint8_t trend, float temperature, float humidity) {
  String message = "";
  // ID Part 1
  message = utils.int2bin(deviceId, 4);
  // channel
  message += utils.int2bin(channel, 2);
  // ID Part 2
  message += utils.int2bin(deviceId >> 4, 2);
  // Low battery warning bit (1 = low batt)
  message += battOk ? "0" : "1";
  // trend ( 0 = cont., 1 = rising, 2 = falling )
  message += utils.int2bin(trend, 2);
  // forced send bit (1 = forced)
  message += manualSend ? "1" : "0";
  // unknown
  message += utils.int2bin(0, 5);
  // humidity
  int h = int(humidity);
  message += utils.int2bin(h, 7);
  // temperature
  int t;
  if (temperature < 0) {
    t = (int((-temperature) * 10.0) & 0xeff) + 0x800;
  } 
  else {
    t = int(temperature * 10.0) & 0xeff;
  }
  message += utils.int2bin(t, 12);

  return message;
}

char rxBuffer[32];
char txBuf[37];

/*
 I  Device Id
 C  Channel
 B  Battery low
 TT Trend
 F  Force send
 t  temperature
 h  humidity
 c  checksum
 */
static void sendEnv() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
#ifdef THS_DEBUG
  if (!isnan(temperature)) {
    Serial.print(F("T: "));
    Serial.print(temperature,1);
  }
  if (!isnan(humidity)) {
    Serial.print(F(" H: "));
    Serial.print(humidity,1);
  }
#endif

  long vcc = utils.getBatteryVoltage();
  bool battOk = vcc > MIN_VCC;
  bool manualSend = false;
  int trend = 0;

  generateEnvMessage(prefs.getDeviceId(), prefs.getChannel(), battOk, manualSend,
                    trend, temperature, humidity).toCharArray(txBuf, 37);
#ifdef THS_DEBUG
  Serial.print(" ");
  Serial.print(txBuf);
  Serial.print("\n");
#endif
  env_sendMessage(txBuf);
} 

static void printEnv() {
#ifdef THS_DEBUG
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (!isnan(temperature)) {
    Serial.print(F("T: "));
    Serial.print(temperature,1);
  }
  if (!isnan(humidity)) {
    Serial.print(F(" H: "));
    Serial.print(humidity,1);
  }
  Serial.print("\n");
#endif
} 

static inline void processCommands() {
  rxBuffer[31] = 0;

#ifdef THS_DEBUG
  while (Serial.available() > 0) {
    Serial.readBytesUntil('\n', rxBuffer, sizeof(rxBuffer) - 1);
    char in = rxBuffer[0];
    switch (in) {
    case 'V':
      Serial.print(F("version: THSens V1.3 2015-01-31\n"));
      break;
    case 'i':
      Serial.print(F("device id "));
      Serial.print(prefs.getDeviceId());
      Serial.print("\n");
      Serial.print(F("channel "));
      Serial.print(prefs.getChannel());
      Serial.print("\n");
      Serial.print(F("tx delay "));
      Serial.print(prefs.getTxInterval());
      Serial.print(" minutes\n");
      Serial.print(F("VCC "));
      Serial.print(utils.getBatteryVoltage()*0.001,2);
      Serial.print("V\n");
      Serial.print(F("CPU temperature "));
      Serial.print(utils.getCPUTemperature(), 1);
      Serial.print("C\n");
      Serial.print("\n");
      break;
    case 'c':
      {
        char *param1 = strtok(&rxBuffer[1], ",");
        char *param2 = strtok(NULL, ",");
        char *param3 = strtok(NULL, ",");
        if (param1 != NULL && param2 != NULL && param3 != NULL) {
          uint8_t d = strtol(param1, NULL, 16);
          uint8_t c = strtol(param2, NULL, 10);
          uint8_t t = strtol(param3, NULL, 10);
          if (d >= 64 || d < 0) {
            Serial.print(F("invalid device id (max 64)\n"));
            break;
          }
          if (c >= 4 || c < 0) {
            Serial.print(F("invalid channel (max 4)\n"));
            break;
          }
          prefs.setDeviceId(d);
          prefs.setChannel(c);
          if (t == 0) {
            Serial.print(F("invalid tx delay\n"));
            Serial.print(t);
            break;
          }
          prefs.setTxInterval(t);
          prefs.save();
        } else {
            Serial.print(F("invalid parameters\n"));
            Serial.print(rxBuffer);
            Serial.print('\n');
            Serial.print(F("parameter 1: "));
            if (param1 != NULL) {
              Serial.print(param1);
            } else {
              Serial.print(F("not found"));
            }
            Serial.print('\n');
            Serial.print(F("parameter 2: "));
            if (param1 != NULL) {
              Serial.print(param2);
            } else {
              Serial.print(F("not found"));
            }
            Serial.print('\n');
            Serial.print(F("parameter 3: "));
            if (param1 != NULL) {
              Serial.print(param3);
            } else {
              Serial.print(F("not found"));
            }
            Serial.print('\n');
        }
      }
      break;
    case 'E':
      sendEnv();
      break;
    case 'S':
      printEnv();
      break;
    case 'r':
      Serial.print(F("random number (0...100) "));
      Serial.print(TrueRandom.random(100));
      Serial.print('\n');
      break;
    case 'H':
    case 'h':
    case '?':
      Serial.print(F("commands:\n"
                     "E: read sensors and send message\n"
                     "S: read sensors\n"
                     "i: show info\n"
                     "r: show random number\n"
                     "V: display version information\n"
                     "cxx,y,z: configure (xx two-digit device id, y one-digit channel, z tx delay in minutes\n"));
      break;
    case '\n':
      break;
    default:
      Serial.print(F("wrong command "));
      Serial.print(in);
      Serial.print('\n');
      break;
    }
  }
#endif
}


void loop() {
  if (last >= prefs.getTxInterval()) {
    last = 0;
    sendEnv();
  }
#ifdef THS_DEBUG
  for (int d = 0; d < 60; d++) {
    if (Serial.available() > 0) {
      processCommands();
    }
    
    uint8_t storeADCSRA = ADCSRA;
    ADCSRA = 0;
    power_adc_disable();
    LowPower.powerDown(SLEEP_500MS, ADC_ON, BOD_OFF);
    noInterrupts();
    timer0_millis += 500;
    interrupts();
    power_adc_enable();
    ADCSRA = storeADCSRA;
    delay(500);

  }
#else
  uint8_t storeADCSRA = ADCSRA;
  ADCSRA = 0;
  power_adc_disable();
  for (uint8_t sleeps = 0; sleeps < 7; sleeps++) {
    LowPower.powerDown(SLEEP_8S, ADC_ON, BOD_OFF);
    noInterrupts();
    timer0_millis += 8000;
    interrupts();
  }
  for (uint8_t sleeps = 0; sleeps < 1; sleeps++) {
    LowPower.powerDown(SLEEP_4S, ADC_ON, BOD_OFF);
    noInterrupts();
    timer0_millis += 4000;
    interrupts();
  }
  power_adc_enable();
  ADCSRA = storeADCSRA;
#endif
  last++;
}

