#ifndef CRCCALCULATOR_H
#define CRCCALCULATOR_H
#include <limits.h>
#include <cstddef>

namespace Belle2 {
  unsigned short CalcCRC16(unsigned short crc16, const char buf[], size_t size);
}
#endif
