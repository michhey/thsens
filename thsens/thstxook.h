// 2015-02-03 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#ifndef THS_TX_OOK
#define THS_TX_OOK

#include "thsprotocol.h"

/*
    Implementation of OOK (on-off-keying) protocol (transmission)
 */

class THSTxOOK {
  public:
    THSTxOOK(int txPin, int syncHi, int syncLo, int zeroHi, int zeroLo, int oneHi, int oneLo);
    THSTxOOK(int txPin, THSProtocol *prot);

    void init();
    void transmitBit(int numHighPulses, int numLowPulses);
    void transmitMessage(String message);

  private:
    const int txPin;
    const int syncHi;
    const int syncLo;
    const int zeroHi;
    const int zeroLo;
    const int oneHi;
    const int oneLo;
};

#endif

