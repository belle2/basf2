#include "daq/slc/copper/FEE.h"

using namespace Belle2;

unsigned int FEE::mask(unsigned int val, int max, int min)
{
  if (max > 0) {
    int bit = 0;
    for (int i = min; i <= max; i++) {
      bit = bit | 0x1 << i;
    }
    val = (bit & val) >> min;
  }
  return val;
}


unsigned int FEE::mask(int max, int min)
{
  if (max > 0) {
    int bit = 0;
    for (int i = min; i <= max; i++) {
      bit = bit | 0x1 << i;
    }
    return bit;
  }
  return 0xFFFFFFFF;
}


