#ifndef CRCCALCULATOR_H
#define CRCCALCULATOR_H
#include <limits.h>
#include <cstddef>

namespace Belle2 {
  /**
   * Function to calculate CRC16
   * Belle2link(HSLB/FEE) trailers.are supposed to contain CRC information.
   * This function will be used to check them in DAQ system.
   */
  unsigned short CalcCRC16(unsigned short crc16, const char buf[], size_t size);
}
#endif
