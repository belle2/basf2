/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/digitization/ECLCompress.h>
#include <ecl/digitization/EclConfiguration.h>

using namespace Belle2;

unsigned int Belle2::ilog2(unsigned int v)  // find the log base 2 of 32-bit v
{
  static const unsigned char MultiplyDeBruijnBitPosition[32] = {
    0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
  };

  v |= v >> 1; // first round down to one less than a power of 2
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;

  return MultiplyDeBruijnBitPosition[(v * 0x07C4ACDDU) >> 27];
}

void ECLBaseCompress::compress(BitStream& out, const int* adc)
{
  int amin = adc[0], amax = adc[0];
  for (unsigned int i = 0; i < EclConfiguration::m_nsmp; i++) {
    amax = std::max(amax, adc[i]);
    amin = std::min(amin, adc[i]);
  }

  amin &= 0x3ffff;
  out.putNBits(amin, 18);
  unsigned int w = ilog2(amax - amin) + 1;
  w &= 0x1f;
  out.putNBits(w, 5);

  for (unsigned int i = 0; i < EclConfiguration::m_nsmp; i++) {
    unsigned int d = adc[i] - amin;
    out.putNBits(d, w);
  }
}

void ECLBaseCompress::uncompress(BitStream& out, int* adc)
{
  unsigned int amin = out.getNBits(18), w = out.getNBits(5);
  for (unsigned int i = 0; i < EclConfiguration::m_nsmp; i++)
    adc[i] = amin + out.getNBits(w);
}

void ECLDeltaCompress::compress(BitStream& out, const int* adc)
{
  int a0 = adc[0], dmin = 0, dmax = 0;
  for (int k = 1; k < EclConfiguration::m_nsmp; k++) {
    int d = adc[k] - a0;
    dmin = std::min(dmin, d);
    dmax = std::max(dmax, d);
    a0 = adc[k];
  }

  unsigned int w = ilog2(std::max(2 * dmax, -2 * dmin)) + 1;
  w &= 0xf;
  out.putNBits(w, 4);

  a0 = adc[0] & 0x3ffff;
  out.putNBits(a0, 18);

  unsigned int base = 1 << (w - 1);
  for (unsigned int i = 1; i < EclConfiguration::m_nsmp; i++) {
    unsigned int d = int(adc[i]) - a0 + base;
    out.putNBits(d, w);
    a0 = adc[i];
  }
}

void ECLDeltaCompress::uncompress(BitStream& out, int* adc)
{
  unsigned int w = out.getNBits(4);
  adc[0] = out.getNBits(18);
  unsigned int base = 1 << (w - 1);
  for (unsigned int i = 1; i < EclConfiguration::m_nsmp; i++) {
    adc[i] = adc[i - 1] + out.getNBits(w) - base;
  }
}
