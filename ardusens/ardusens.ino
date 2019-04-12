
// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

/*
 *  wireless tempereature/humidity sensor with low power consumption
 *
 */

#include <SensorTransmitter.h>
#include <DHT.h>
#include <Wire.h>
#include <LowPower.h>
#include <TrueRandom.h>
#include <EEPROM.h>
#include <avr/power.h>

#include "thsconfig.h"
#include "thsutils.h"

#define AS_TYPE_MOISTURE      1
#define AS_TYPE_DOOR          2
#define AS_TYPE_LIGHT_HIRANGE 3
#define AS_TYPE_LIGHT_HIRES   4
#define AS_TYPE_WATER         5
#define AS_TYPE_TEMPERATURE   6
#define AS_TYPE_REED_GAS      7
#define AS_TYPE_VOLTAGE       8
#define AS_TYPE_HUMIDITY      9

#define AS_BATT_STATE_OPTIMAL 3
#define AS_BATT_STATE_OK      2
#define AS_BATT_STATE_CHANGE  1
#define AS_BATT_STATE_BAD     0

#define DEVICE_ID    0
#define TX_INTERVAL  15

THSUtils utils;
asTransmitter voltageTx(AS_TYPE_VOLTAGE, DEVICE_ID, TRANSMITTER_PIN);          // (DeviceType, DeviceID, OutputPin)
asTransmitter temperatureTx(AS_TYPE_TEMPERATURE, DEVICE_ID, TRANSMITTER_PIN);  // (DeviceType, DeviceID, OutputPin)
asTransmitter humidityTx(AS_TYPE_HUMIDITY, DEVICE_ID, TRANSMITTER_PIN);        // (DeviceType, DeviceID, OutputPin)
DHT dht(DHT_PIN, DHTTYPE);
// kind of a hack (timer0_millis is the arduino core libs internal millis counter variable)
extern long timer0_millis;
boolean battStateOk;

#ifdef THS_DEBUG
static char rxBuffer[32];
#endif

void setup() {
  battStateOk = true;
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

  // init tx indicator led
#ifdef TX_INDICATOR_LED_PIN
  pinMode(TX_INDICATOR_LED_PIN, OUTPUT); 
  digitalWrite(TX_INDICATOR_LED_PIN, LOW);
#endif

  // temperature/humidity sensor dht22
  dht.begin();
  
  temperatureTx.setRepeat(6);
  humidityTx.setRepeat(6);
  voltageTx.setRepeat(6);

  sendEnv(false);
}

static void sendEnv(boolean manualSend) {
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
  if (battStateOk) {
    battStateOk = utils.getBatteryVoltage() > MIN_BATTERY_VOLTAGE;
  } else {
    battStateOk = utils.getBatteryVoltage() > (MIN_BATTERY_VOLTAGE + BATTERY_VOLTAGE_HYSTERESIS);
  }
  
  uint8_t battState = battStateOk ? AS_BATT_STATE_OK : AS_BATT_STATE_BAD;
  
  uint16_t tempVal = (uint16_t)((int)(temperature * 10) + 0x8000);
  uint16_t humVal  = (uint16_t)((int)(humidity * 10) + 0x8000);
  
  temperatureTx.send(tempVal, AS_BATT_STATE_OK, manualSend);
  delay(50);
  humidityTx.send(humVal, AS_BATT_STATE_OK, manualSend);
  delay(50);
  voltageTx.send(utils.getBatteryVoltage(), battState, manualSend);

#ifdef THS_DEBUG
  Serial.print(" tmp ");
  Serial.println(tempVal);
  Serial.print(" hum ");
  Serial.println(humVal);
  Serial.print(" bat ");
  Serial.println(utils.getBatteryVoltage());
  Serial.flush();
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
  Serial.flush();
} 

static inline void processCommands() {
  rxBuffer[31] = 0;

  while (Serial.available() > 0) {
    Serial.readBytesUntil('\n', rxBuffer, sizeof(rxBuffer) - 1);
    char in = rxBuffer[0];
    switch (in) {
    case 'V':
      Serial.print(F("version: ArduSens V0.1.0 " __DATE__ "\n"));
      break;
    case 'i':
      Serial.print(F("device id "));
      Serial.println(DEVICE_ID);
      Serial.print(F("VCC "));
      Serial.print(utils.getBatteryVoltage()*0.001,2);
      Serial.print("V\n");
      Serial.print(F("CPU temperature "));
      Serial.print(utils.getCPUTemperature(), 1);
      Serial.print("C\n");
      Serial.print("\n");
      Serial.flush();
      break;
    case 'E':
      sendEnv(true);
      break;
    case 'S':
      printEnv();
      break;
    case 'r':
      Serial.print(F("random number (0...100) "));
      Serial.print(TrueRandom.random(100));
      Serial.print('\n');
      Serial.flush();
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
      Serial.flush();
      break;
    case '\n':
      break;
    default:
      Serial.print(F("wrong command "));
      Serial.print(in);
      Serial.print('\n');
      Serial.flush();
      break;
    }
  }
}
#endif


void loop() {
  sendEnv(false);
#ifdef THS_DEBUG
  for (int seconds = 60 * TX_INTERVAL; seconds ; seconds--) {
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
  unsigned int seconds = 60 * TX_INTERVAL;
  
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

