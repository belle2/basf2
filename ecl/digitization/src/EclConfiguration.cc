/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/digitization/shaperdsp.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <vector>
#include <cmath>

using namespace Belle2;
using namespace Belle2::ECL;
using namespace std;

// define the constexpr here to make clang happy. Could be a bug or some strict
// adherence to the standard. I guess it's just a different interpretation of
// how to use these values by clang.
// http://stackoverflow.com/questions/28264279/undefined-reference-when-accessing-static-constexpr-float-member
constexpr double EclConfiguration::s_clock;
constexpr double EclConfiguration::m_step;
double EclConfiguration::m_rf = -1;
double EclConfiguration::m_tick = -1;

double EclConfiguration::getRF()
{
  /**< Default RF value is 508.876 MHz,
   *   see www-linac.kek.jp/linac-com/report/skb-tdr/, ch. 6 */
  if (m_rf < 0) {
    DBObjPtr<Belle2::HardwareClockSettings> clock_info("HardwareClockSettings");
    m_rf = clock_info->getAcceleratorRF();
  }
  return m_rf;
}

double EclConfiguration::getTick()
{
  return 24.*12. / getRF();
}

void EclConfiguration::signalsample_t::InitSample(const double* MPd, double u)
{
  const int N = m_ns * m_nl;
  vector<double> MP(MPd, MPd + 10);
  ShaperDSP_t dsp(MP, u);
  dsp.settimestride(m_step / m_ns);
  dsp.fillarray(0.0, N, m_ft);

  double sum = 0;
  for (int i = 0; i < N; i++) sum += m_ft[i];
  m_sumscale = m_ns / sum;
}


void EclConfiguration::signalsample_t::InitSample(const float* MP, double u)
{
  double MPd[10];
  for (int i = 0; i < 10; i++) MPd[i] = MP[i];
  InitSample(MPd, u);
}

void EclConfiguration::adccounts_t::AddHit(const double a, const double t0, const EclConfiguration::signalsample_t& s)
{
  total += s.Accumulate(a, t0, c);
}

double EclConfiguration::signalsample_t::Accumulate(const double a, const double t0, double* s) const
{
  // input parameters
  // a -- signal amplitude
  // t -- signal offset
  // output parameter
  // s -- output array with added signal
  const double itick = getRF() / s_clock;  // reciprocal to avoid division in usec^-1 (has to be evaluated at compile time)
  const double  tlen = m_nl - 1.0 / m_ns;   // length of the sampled signal in ADC clocks units
  const double  tmax = m_tmin + m_nsmp - 1; // upper range of the fit region

  double t = t0 * itick; // rescale time in usec to ADC clocks
  double x0 = t, x1 = t + tlen;

  if (x0 > tmax) return 0; // signal starts after the upper range of output array -- do nothing
  if (x0 < m_tmin) {
    if (x1 < m_tmin) return 0; // signal ends before lower range of output array -- do nothing
    x0 = m_tmin; // adjust signal with range of output array
  }

  int imax = m_nsmp; // length of sampled signal is long enough so
  // the last touched element is the last element
  // of the output array
  if (x1 < tmax) { // if initial time is too early we need to adjust
    // the last touched element of output array to avoid
    // out-of-bound situation in m_ft
    imax = x1 - m_tmin; // imax is always positive so floor can be
    // replace by simple typecast
    imax += 1; // like s.end()
  }

  double imind = ceil(x0 - m_tmin); // store result in double to avoid int->double conversion below
  // the ceil function today at modern CPUs is surprisingly fast (before it was horribly slow)
  int imin = imind; // starting point to fill output array
  double w = ((m_tmin - t) + imind) * double(m_ns);
  int jmin = w; // starting point in sampled signal array
  w -= jmin;

  // use linear interpolation between samples. Since signal samples
  // are aligned with output samples only two weights are need to
  // calculate to fill output array
  const double w1 = a * w, w0 = a - w1;
  double sum = 0;
  for (int i = imin, j = jmin; i < imax; i++, j += m_ns) {
    double amp = w0 * m_ft[j] + w1 * m_ft[j + 1];
    s[i] += amp;
    sum  += amp;
  }
  return sum * m_sumscale;
}
