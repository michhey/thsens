// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#ifndef THS_UTILS
#define THS_UTILS

class THSUtils {
  public:
    long getBatteryVoltage();
    float getCPUTemperature();
    String int2bin(int value, int numbits);
    int bin2int(String message);
};

#endif

