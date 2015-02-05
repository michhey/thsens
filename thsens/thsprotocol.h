// 2015-02-03 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#ifndef THS_PROTOCOL
#define THS_PROTOCOL

/*
    base class for protocol implementations
 */

class THSProtocol {
  public:
    THSProtocol(int syncHiIn, int syncLoIn, int zeroHiIn, int zeroLoIn, int oneHiIn, int oneLoIn)
     : syncHi(syncHiIn), syncLo(syncLoIn),
       zeroHi(zeroHiIn), zeroLo(zeroLoIn), oneHi(oneHiIn), oneLo(oneLoIn) {
    }

    // encode data to binary representation (string with 0/1 characters as bits)  
    virtual String encode() = 0;

    // protocol timing (all in microseconds)
    // duration of SYNC field high period
    const int syncHi;
    // duration of SYNC field low period
    const int syncLo;
    // duration of data bit zero high period
    const int zeroHi;
    // duration of data bit zero low period
    const int zeroLo;
    // duration of data bit one high period
    const int oneHi;
    // duration of data bit one low period
    const int oneLo;

  protected:
    String message;
};

#endif

