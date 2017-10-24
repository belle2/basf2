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
#include <math.h>
#include <assert.h>
#include <stdio.h>

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

struct width_t {
  unsigned char w0, w1, w2, w3;
};

width_t widths_scale025[] = {
  {7, 9, 18, 32},//  0 7.72447
  {5, 7, 18, 32},//  1 5.45839
  {5, 7, 18, 32},//  2 5.41033
  {5, 7, 18, 32},//  3 5.32805
  {5, 7, 18, 32},//  4 5.24319
  {4, 6, 18, 32},//  5 5.01064
  {4, 6, 18, 32},//  6 4.75667
  {4, 6, 18, 32},//  7 4.53016
  {4, 6, 18, 32},//  8 4.33823
  {3, 5, 18, 32},//  9 4.08838
  {3, 5, 18, 32},// 10 3.7205
  {3, 4, 18, 32},// 11 3.41795
  {2, 4, 18, 32},// 12 3.17582
  {2, 4, 18, 32},// 13 2.76922
  {2, 3, 18, 32},// 14 2.42437
  {2, 3, 18, 32},// 15 2.20985
  {2, 3, 18, 32},// 16 2.09761
  {2, 3, 18, 32},// 17 2.04576
  {2, 3, 18, 32},// 18 2.02288
  {2, 4, 18, 32},// 19 2.01152
  {2, 4, 18, 32},// 20 2.00643
  {2, 4, 18, 32},// 21 2.00386
  {2, 4, 18, 32},// 22 2.00244
  {2, 4, 18, 32},// 23 2.00171
  {2, 4, 18, 32},// 24 2.0012
  {2, 4, 18, 32},// 25 2.00081
  {2, 4, 18, 32},// 26 2.00055
  {2, 4, 18, 32},// 27 2.00036
  {2, 4, 18, 32},// 28 2.00023
  {2, 3, 18, 32},// 29 2.00012
  {2, 3, 18, 32} // 30 2.00004
};

void stream_int(BitStream& OUT, int x, const width_t& w)
{
  int ax = abs(x), m0 = 1 << (w.w0 - 1), m1 = 1 << (w.w1 - 1), m2 = 1 << (w.w2 - 1);
  if (ax < m0) {
    OUT.putNBits(x, w.w0);
  } else if (ax < m1) {
    OUT.putNBits((x << w.w0) | m0, w.w1 + w.w0);
  } else if (ax < m2) {
    OUT.putNBits((m1 << w.w0) | m0, w.w1 + w.w0);
    OUT.putNBits(x, w.w2);
  } else {
    OUT.putNBits((m1 << w.w0) | m0, w.w1 + w.w0);
    OUT.putNBits(m2, w.w2);
    OUT.putNBits(x, w.w3);
  }
}

int fetch_int(BitStream& IN, const width_t& w)
{
  int m0 = 1 << (w.w0 - 1), m1 = 1 << (w.w1 - 1), m2 = 1 << (w.w2 - 1);
  int t = IN.getNBits(w.w0);
  if (t == m0) {
    t = IN.getNBits(w.w1);
    if (t == m1) {
      t = IN.getNBits(w.w2);
      if (t == m2) {
        t = IN.getNBits(w.w3);
      } else {
        t = (t << (32 - w.w2)) >> (32 - w.w2);
      }
    } else {
      t = (t << (32 - w.w1)) >> (32 - w.w1);
    }
  } else {
    t = (t << (32 - w.w0)) >> (32 - w.w0);
  }
  return t;
}

extern "C" {
  void e10_31(const double* I, double* O);
  void e01_31(const double* I, double* O);
}

void ECLDCTCompress::compress(BitStream& OUT, const int* a)
{
  const double scale = 0.25;
  const int N = EclConfiguration::m_nsmp;
  double buf[N], out[N];
  for (int k = 0; k < N; k++) buf[k] = a[k];
  e10_31(buf, out);
  for (int k = 0; k < N; k++) out[k] *= 1.0 / (2 * N);

  int km = N;
  for (; km > 16; --km) if (lrint(out[km - 1]*scale) != 0) break;
  OUT.putNBits(N - km, 4);
  out[0] -= 3144;
  for (int k = 0; k < km; k++) {
    int t = lrint(out[k] * scale);
    stream_int(OUT, t, widths_scale025[k]);
  }
}

void ECLDCTCompress::uncompress(BitStream& in, int* adc)
{
  const double scale = 0.25, iscale = 1 / scale;
  const int N = EclConfiguration::m_nsmp;
  int nz = in.getNBits(4);
  for (int i = 0; i < N - nz; i++) adc[i] = fetch_int(in, widths_scale025[i]);
  for (int i = N - nz; i < N; i++) adc[i] = 0;

  double c[N], out[N];
  for (int k = 0; k < N; k++) c[k] = adc[k] * iscale;
  c[0] += 3144;

  e01_31(c, out);

  for (int k = 0; k < N; k++) adc[k] = lrint(out[k]);
}

ECLCompress* selectAlgo(int compAlgo)
{
  ECLCompress* comp = NULL;
  if (compAlgo == 1) {
    comp = new ECLBaseCompress;
  } else if (compAlgo == 2) {
    comp = new ECLDeltaCompress;
  } else if (compAlgo == 3) {
    comp = new ECLDCTCompress;
  }
  return comp;
}
