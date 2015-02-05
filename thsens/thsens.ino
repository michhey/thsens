// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

/*
 *  wireless tempereature/humidity sensor with low power consumption
 *
 */

#include <DHT.h>
#include <Wire.h>
#include <LowPower.h>
#include <TrueRandom.h>
#include <EEPROM.h>
#include <avr/power.h>

#include "thsconfig.h"
#include "thspreferences.h"
#include "thsutils.h"
#include "thsprotocolect.h"
#include "thstxook.h"

THSPreferences prefs;
THSUtils utils;
THSProtocolECT protECT;
THSTxOOK txOOK(TRANSMITTER_PIN, &protECT);
DHT dht(DHT_PIN, DHTTYPE);
// kind of a hack (timer0_millis is the arduino core libs internal millis counter variable)
extern long timer0_millis;

#ifdef THS_DEBUG
static char rxBuffer[32];
#endif

void setup() {  
#ifdef THS_DEBUG
  // initialize serial:
  Serial.begin(57600);
  Serial.setTimeout(10);
  rxBuffer[0] = 0;
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

  protECT.deviceId = prefs.getDeviceId();
  protECT.channel = prefs.getChannel();
  protECT.manualSend = false;

  txOOK.init();

  // init tx indicator led
#ifdef TX_INDICATOR_LED_PIN
  pinMode(TX_INDICATOR_LED_PIN, OUTPUT); 
  digitalWrite(TX_INDICATOR_LED_PIN, LOW);
#endif

  // temperature/humidity sensor dht22
  dht.begin();

  prefs.load();
  
  sendEnv();
}

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

  protECT.batteryOk = utils.getBatteryVoltage() > MIN_BATTERY_VOLTAGE;
  protECT.temperature = temperature;
  protECT.humidity = humidity;
  String message = protECT.encode();
  txOOK.transmitMessage(message);

#ifdef THS_DEBUG
  Serial.print(" ");
  Serial.print(message);
  Serial.print("\n");
#endif
} 

#ifdef THS_DEBUG
static void printEnv() {
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
} 

static inline void processCommands() {
  rxBuffer[31] = 0;

  while (Serial.available() > 0) {
    Serial.readBytesUntil('\n', rxBuffer, sizeof(rxBuffer) - 1);
    char in = rxBuffer[0];
    switch (in) {
    case 'V':
      Serial.print(F("version: THSens V0.1.3 " __DATE__ "\n"));
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
          protECT.deviceId = prefs.getDeviceId();
          protECT.channel = prefs.getChannel();
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
      protECT.manualSend = true;
      sendEnv();
      protECT.manualSend = false;
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
}
#endif


void loop() {
  sendEnv();
#ifdef THS_DEBUG
  for (int seconds = 60 * prefs.getTxInterval(); seconds ; seconds--) {
    // sleep minimum available time with WDT to test the algorithm, but
    // try not to disturb debugging (UART is disabled while in powerdown mode)
    uint8_t storeADCSRA = ADCSRA;
    ADCSRA = 0;
    power_adc_disable();
    LowPower.powerDown(SLEEP_30MS, ADC_ON, BOD_OFF);
    noInterrupts();
    timer0_millis += 30;
    interrupts();
    power_adc_enable();
    ADCSRA = storeADCSRA;
    delay(485);

    if (Serial.available() > 0) {
      processCommands();
    }
    delay(500);

    if (Serial.available() > 0) {
      processCommands();
    }
  }
#else
  uint8_t storeADCSRA = ADCSRA;
  ADCSRA = 0;
  power_adc_disable();
  unsigned int seconds = 60 * prefs.getTxInterval();
  
  // maximum sleep time with WDT is 8 seconds
  for (unsigned int sleepInterval8s = seconds / 8; sleepInterval8s; sleepInterval8s--) {
    LowPower.powerDown(SLEEP_8S, ADC_ON, BOD_OFF);
    noInterrupts();
    timer0_millis += 8000;
    interrupts();
  }
  // wait remaining seconds in one second interval
  for (unsigned int sleepInterval1s = seconds % 8; sleepInterval1s; sleepInterval1s--) {
    LowPower.powerDown(SLEEP_1S, ADC_ON, BOD_OFF);
    noInterrupts();
    timer0_millis += 1000;
    interrupts();
  }
  power_adc_enable();
  ADCSRA = storeADCSRA;
#endif
}

