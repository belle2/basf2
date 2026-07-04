/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/digitization/BitStream.h>
#include <ecl/digitization/ECLCompressUp.h>
#include <ecl/digitization/EclConfigurationUp.h>

#include <algorithm>
#include <math.h>

using namespace Belle2;
using namespace ECL;

namespace {
  unsigned int ilog2_up(unsigned int v)  // find the log base 2 of 32-bit v
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
}

void ECLBaseCompressUp::compress(BitStream& out, const int* adc)
{
  int amin = adc[0], amax = adc[0];
  for (unsigned int i = 0; i < EclConfigurationUp::m_nsmp; i++) {
    amax = std::max(amax, adc[i]);
    amin = std::min(amin, adc[i]);
  }

  amin &= 0x3ffff;
  out.putNBits(amin, 18);
  unsigned int w = ilog2_up(amax - amin) + 1;
  w &= 0x1f;
  out.putNBits(w, 5);

  for (unsigned int i = 0; i < EclConfigurationUp::m_nsmp; i++) {
    unsigned int d = adc[i] - amin;
    out.putNBits(d, w);
  }
}

void ECLBaseCompressUp::uncompress(BitStream& out, int* adc)
{
  unsigned int amin = out.getNBits(18), w = out.getNBits(5);
  for (unsigned int i = 0; i < EclConfigurationUp::m_nsmp; i++)
    adc[i] = amin + out.getNBits(w);
}

void ECLDeltaCompressUp::compress(BitStream& out, const int* adc)
{
  int a0 = adc[0], dmin = 0, dmax = 0;
  for (int k = 1; k < EclConfigurationUp::m_nsmp; k++) {
    int d = adc[k] - a0;
    dmin = std::min(dmin, d);
    dmax = std::max(dmax, d);
    a0 = adc[k];
  }

  unsigned int w = ilog2_up(std::max(2 * dmax, -2 * dmin)) + 1;
  w &= 0xf;
  out.putNBits(w, 4);

  a0 = adc[0] & 0x3ffff;
  out.putNBits(a0, 18);

  unsigned int base = 1 << (w - 1);
  for (unsigned int i = 1; i < EclConfigurationUp::m_nsmp; i++) {
    unsigned int d = int(adc[i]) - a0 + base;
    out.putNBits(d, w);
    a0 = adc[i];
  }
}

void ECLDeltaCompressUp::uncompress(BitStream& out, int* adc)
{
  unsigned int w = out.getNBits(4);
  adc[0] = out.getNBits(18);
  unsigned int base = 1 << (w - 1);
  for (unsigned int i = 1; i < EclConfigurationUp::m_nsmp; i++) {
    adc[i] = adc[i - 1] + out.getNBits(w) - base;
  }
}

ECLCompressUp* Belle2::ECL::selectAlgoUp(int compAlgo)
{
  ECLCompressUp* comp = nullptr;
  if (compAlgo == 1) {
    comp = new ECLBaseCompressUp;
  } else if (compAlgo == 2) {
    comp = new ECLDeltaCompressUp;
  }
  return comp;
}
