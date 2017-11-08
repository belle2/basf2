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

namespace {
  /** Based on phase 3 MC bit widths for prefix coding for all DCT
   * coefficients with scale factor s=0.25 used in quantization process
   * which provides the best compression factor. Comment in each line
   * shows the average number of bits to encode the coefficient
   */
  width_t widths_scale025[] = {
    {7, 9, 18, 32},// 7.72447
    {5, 7, 18, 32},// 5.45839
    {5, 7, 18, 32},// 5.41033
    {5, 7, 18, 32},// 5.32805
    {5, 7, 18, 32},// 5.24319
    {4, 6, 18, 32},// 5.01064
    {4, 6, 18, 32},// 4.75667
    {4, 6, 18, 32},// 4.53016
    {4, 6, 18, 32},// 4.33823
    {3, 5, 18, 32},// 4.08838
    {3, 5, 18, 32},// 3.7205
    {3, 4, 18, 32},// 3.41795
    {2, 4, 18, 32},// 3.17582
    {2, 4, 18, 32},// 2.76922
    {2, 3, 18, 32},// 2.42437
    {2, 3, 18, 32},// 2.20985
    {2, 3, 18, 32},// 2.09761
    {2, 3, 18, 32},// 2.04576
    {2, 3, 18, 32},// 2.02288
    {2, 4, 18, 32},// 2.01152
    {2, 4, 18, 32},// 2.00643
    {2, 4, 18, 32},// 2.00386
    {2, 4, 18, 32},// 2.00244
    {2, 4, 18, 32},// 2.00171
    {2, 4, 18, 32},// 2.0012
    {2, 4, 18, 32},// 2.00081
    {2, 4, 18, 32},// 2.00055
    {2, 4, 18, 32},// 2.00036
    {2, 4, 18, 32},// 2.00023
    {2, 3, 18, 32},// 2.00012
    {2, 3, 18, 32} // 2.00004
  };

  /** Based on phase 2 MC bit widths for prefix coding for all DCT
   * coefficients with scale factor s=1 used in quantization process
   * which provides the best compression factor. Comment in each line
   * shows the average number of bits to encode the coefficient
   */
  width_t widths_phs2_scale10[] = {
    {5, 7, 9, 32},// 5.82104
    {4, 6, 8, 32},// 4.76806
    {4, 6, 8, 32},// 4.70815
    {4, 6, 8, 32},// 4.61517
    {3, 5, 7, 32},// 4.42656
    {3, 5, 7, 32},// 4.22157
    {3, 5, 7, 32},// 4.01412
    {3, 5, 7, 32},// 3.80959
    {2, 4, 6, 32},// 3.60224
    {2, 4, 6, 32},// 3.31705
    {2, 4, 6, 32},// 3.03457
    {2, 3, 5, 32},// 2.71501
    {2, 3, 5, 32},// 2.45094
    {2, 3, 5, 32},// 2.25788
    {2, 3, 5, 32},// 2.13303
    {2, 3, 5, 32},// 2.06428
    {2, 3, 5, 32},// 2.02847
    {2, 3, 5, 32},// 2.01253
    {1, 2, 4, 32},// 1.86085
    {1, 2, 4, 32},// 1.68465
    {1, 2, 4, 32},// 1.53003
    {1, 2, 4, 32},// 1.38031
    {1, 2, 4, 32},// 1.27103
    {1, 2, 4, 32},// 1.18264
    {1, 2, 4, 32},// 1.11546
    {1, 2, 4, 32},// 1.07223
    {1, 2, 4, 32},// 1.04641
    {1, 2, 4, 32},// 1.03003
    {1, 2, 4, 32},// 1.01772
    {1, 2, 3, 32},// 1.01304
    {1, 2, 4, 32},// 1.0107
  };
}

/** put integer "x" to the stream OUT with a priory knowledge how "x"
 *  is distributed encoded in "w".
 * @param OUT output bit stream
 * @param x integer to compress and stream out
 * @param w progessive bit widths
 */
void stream_int(BitStream& OUT, int x, const width_t& w)
{
  int ax = abs(x), m0 = 1 << (w.w0 - 1), m1 = 1 << (w.w1 - 1), m2 = 1 << (w.w2 - 1);
  if (ax < m0) { // integer fits into w0 bits
    OUT.putNBits(x, w.w0);
  } else if (ax < m1) {// integer fits into w1 bits
    OUT.putNBits((x << w.w0) | m0, w.w1 + w.w0); // first stream prefix showing that we are switching to the next bit width format
  } else if (ax < m2) {// integer fits into w2 bits
    OUT.putNBits((m1 << w.w0) | m0, w.w1 + w.w0);
    OUT.putNBits(x, w.w2);
  } else {// integer fits into w3 bits
    OUT.putNBits((m1 << w.w0) | m0, w.w1 + w.w0);
    OUT.putNBits(m2, w.w2);
    OUT.putNBits(x, w.w3);
  }
}

/** Extract integer from the stream OUT with a priory knowledge how it
 *  is distributed encoded in "w".
 * @param IN input bit stream
 * @param w progessive bit widths
 */
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
  /** DCT-II or "the" DCT transformation of 31-point signal
   * @param I input signal with amplitudes
   * @param O output coefficients of the transformation
   */
  void e10_31(const double* I, double* O);

  /** DCT-III or "the inverse" DCT transformation of 31-point signal
   * @param I input coefficients
   * @param O output signal amplitudes
   */
  void e01_31(const double* I, double* O);
}

ECLDCTCompress::ECLDCTCompress(double scale, double c0, width_t* w): m_scale(scale), m_c0(c0), m_widths(w) {}

void ECLDCTCompress::compress(BitStream& OUT, const int* a)
{
  const int N = EclConfiguration::m_nsmp;
  double buf[N], out[N];
  for (int k = 0; k < N; k++) buf[k] = a[k];
  e10_31(buf, out);
  for (int k = 0; k < N; k++) out[k] *= 1.0 / (2 * N);

  int km = N;
  for (; km > 16; --km) if (lrint(out[km - 1]*m_scale) != 0) break;
  OUT.putNBits(N - km, 4);
  out[0] -= m_c0;
  for (int k = 0; k < km; k++) {
    int t = lrint(out[k] * m_scale);
    stream_int(OUT, t, m_widths[k]);
  }
}

void ECLDCTCompress::uncompress(BitStream& in, int* adc)
{
  const double iscale = 1 / m_scale;
  const int N = EclConfiguration::m_nsmp;
  int nz = in.getNBits(4);
  for (int i = 0; i < N - nz; i++) adc[i] = fetch_int(in, m_widths[i]);
  for (int i = N - nz; i < N; i++) adc[i] = 0;

  double c[N], out[N];
  for (int k = 0; k < N; k++) c[k] = adc[k] * iscale;
  c[0] += m_c0;

  e01_31(c, out);

  for (int k = 0; k < N; k++) adc[k] = lrint(out[k]);
}

ECLCompress* Belle2::selectAlgo(int compAlgo)
{
  ECLCompress* comp = NULL;
  if (compAlgo == 1) {
    comp = new ECLBaseCompress;
  } else if (compAlgo == 2) {
    comp = new ECLDeltaCompress;
  } else if (compAlgo == 3) {
    comp = new ECLDCTCompress(1, 3012, widths_phs2_scale10);
  } else if (compAlgo == 4) {
    comp = new ECLDCTCompress(0.25, 3144, widths_scale025);
  }
  return comp;
}
