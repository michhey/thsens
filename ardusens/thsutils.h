// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#ifndef THS_UTILS
#define THS_UTILS

class THSUtils {
  public:
    long getBatteryVoltage();
    float getCPUTemperature();

    // convert numbits bits of an int value into a string with binary numbers
    // e.g. value 5 with 4 bits -> "0101"
    static String int2bin(int value, int numbits) {
      String message = "";
      int mask = 1 << (numbits - 1);
      for (int i = 0; i < numbits; i++) {
        if (value & mask) {
          message += "1";
        } 
        else {
          message += "0";
        }
        mask >>= 1;
      }
    
      return message;
    };

    // convert a string with binary numbers into int
    // e.g. "0101" -> 5
    static int bin2int(String message) {
      int value = 0;
    
      for (unsigned int pos = 0; pos < message.length(); pos++) {
        value <<= 1;
        if (message.charAt(pos) == '1') {
          value |= 1;
        }
      }
      return value;
    };
};

#endif

