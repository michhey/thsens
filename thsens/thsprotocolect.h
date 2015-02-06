// 2015-02-03 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#ifndef THS_PROTOCOL_ECT
#define THS_PROTOCOL_ECT

#include "thsprotocol.h"

/*
    EuroChron/Tchibo weather sensor protocol implementations
 */

class THSProtocolECT : public THSProtocol {
  public:
    THSProtocolECT();

    // encode data to binary representation (string with 0/1 characters as bits)  
    String encode();
    
    uint8_t deviceId;
    uint8_t channel;
    boolean batteryOk;
    boolean manualSend;
    float temperature;
    float humidity;
    
};

#endif

