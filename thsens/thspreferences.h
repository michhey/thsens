// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#ifndef THS_PREFERENCES
#define THS_PREFERENCES

class THSPreferences {
  public:
    // load preferences from eeprom
    //   (initializes eeprom if novalid preferences found)
    void load();

    // save preferences
    void save();

    // Device Id
    //    unique device id (forms unique address with channel)
    // valid values: 0-63
    uint8_t getDeviceId()
    {
      return deviceId;
    };
    void setDeviceId(uint8_t deviceId)
    {
      this->deviceId = deviceId;
    };

    // Channel
    // valid values: 0-3
    uint8_t getChannel()
    {
      return channel;
    };
    void setChannel(uint8_t channel)
    {
      this->channel = channel;
    };
  
    // txInterval
    //    transmission interval in minutes
    // valid values: 1-255
    uint8_t getTxInterval()
    {
      return txInterval;
    };
    void setTxInterval(uint8_t txInterval)
    {
      this->txInterval = txInterval;
    };
  
  private:
    uint8_t deviceId;
    uint8_t channel;
    uint8_t txInterval;
    
    const static int EE_PREF_ADDR_MAGIC1 = 0;
    const static int EE_PREF_ADDR_MAGIC2 = 1;
    const static int EE_PREF_ADDR_DEVICE_ID = 2;
    const static int EE_PREF_ADDR_CHANNEL = 3;
    const static int EE_PREF_ADDR_TX_INTERVAL = 4;
    const static uint8_t EE_PREF_MAGIC1 = 'E';
    const static uint8_t EE_PREF_MAGIC2 = 'S';
};

#endif

