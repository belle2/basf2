/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/digitization/EclConfigurationUp.h>

/* ECL headers. */
#include <ecl/digitization/shaperdspUp.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>

/* C++ headers. */
#include <cmath>
#include <vector>

using namespace Belle2;
using namespace Belle2::ECL;

// define the constexpr here to make clang happy. Could be a bug or some strict
// adherence to the standard. I guess it's just a different interpretation of
// how to use these values by clang.
// http://stackoverflow.com/questions/28264279/undefined-reference-when-accessing-static-constexpr-float-member
constexpr double EclConfigurationUp::c_clock;
constexpr double EclConfigurationUp::c_step;
double EclConfigurationUp::s_rf = -1;

double EclConfigurationUp::getRF()
{
  /**< Default RF value is 508.876 MHz,
   *   see www-linac.kek.jp/linac-com/report/skb-tdr/, ch. 6 */
  if (s_rf < 0) {
    DBObjPtr<Belle2::HardwareClockSettings> clock_info("HardwareClockSettings");
    // Convert RF from GHz to MHz
    s_rf = clock_info->getAcceleratorRF() * 1e3;
  }
  return s_rf;
}

std::array<double, EclConfigurationUp::c_nWaveformParameters>
EclConfigurationUp::getDefaultWaveformParameters()
{
  const double timeToUnits = getRF() * c_step / c_clock;

  return {
    c_tauD * timeToUnits,
    c_tau0 * timeToUnits,
    timeToUnits / c_omega0,
    c_tau1 * timeToUnits,
    timeToUnits / c_omega1,
    c_tauI * timeToUnits,
    c_fractionI,
    c_tauS * timeToUnits,
    c_tauS1 * timeToUnits,
    c_alpha,
    c_timeShift * timeToUnits,
    c_eta,
    c_amplitudeNorm,
    c_timeOffset
  };
}

void EclConfigurationUp::SignalSample::initSample(const double* waveformParameters, double unitScale)
{
  const int nSignalSamples = c_nSamplesPerAdcClock * c_nSignalAdcClocks;
  std::vector<double> MP(waveformParameters, waveformParameters + 14);
  ShaperDSPUp_t dsp(MP, unitScale);
  dsp.settimestride(c_step / c_nSamplesPerAdcClock);
  dsp.fillarray(0.0, nSignalSamples, m_signalShape);

  double sum = 0;
  for (int i = 0; i < nSignalSamples; i++) sum += m_signalShape[i];
  m_sumScale = c_nSamplesPerAdcClock / sum;
}


void EclConfigurationUp::SignalSample::initSample(const float* waveformParameters, double unitScale)
{
  double MPd[14];
  for (int i = 0; i < 14; i++) MPd[i] = waveformParameters[i];
  initSample(MPd, unitScale);
}

void EclConfigurationUp::AdcCounts::addHit(double amplitude, double timeOffset,
                                           const EclConfigurationUp::SignalSample& signalSample)
{
  m_total += signalSample.accumulate(amplitude, timeOffset, m_adcCounts);
}

double EclConfigurationUp::SignalSample::accumulate(double amplitude, double timeOffset, double* signal) const
{
  // input parameters
  // amplitude -- signal amplitude
  // timeOffset -- signal offset
  // output parameter
  // signal -- output array with added signal
  const double itick = getRF() / c_clock;  // reciprocal to avoid division in usec^-1 (has to be evaluated at compile time)
  const double tlen = c_nSignalAdcClocks - 1.0 / c_nSamplesPerAdcClock;   // length of the sampled signal in ADC clocks units
  const double tmax = c_fitTimeMin + c_nAdcSamples - 1; // upper range of the fit region

  double t = timeOffset * itick; // rescale time in usec to ADC clocks
  double x0 = t, x1 = t + tlen;

  if (x0 > tmax) return 0; // signal starts after the upper range of output array -- do nothing
  if (x0 < c_fitTimeMin) {
    if (x1 < c_fitTimeMin) return 0; // signal ends before lower range of output array -- do nothing
    x0 = c_fitTimeMin; // adjust signal with range of output array
  }

  int imax = c_nAdcSamples; // length of sampled signal is long enough so
  // the last touched element is the last element
  // of the output array
  if (x1 < tmax) { // if initial time is too early we need to adjust
    // the last touched element of output array to avoid
    // out-of-bound situation in m_signalShape
    imax = x1 - c_fitTimeMin; // imax is always positive so floor can be
    // replace by simple typecast
    imax += 1; // like s.end()
  }

  double imind = std::ceil(x0 - c_fitTimeMin); // store result in double to avoid int->double conversion below
  // the ceil function today at modern CPUs is surprisingly fast (before it was horribly slow)
  int imin = imind; // starting point to fill output array
  double w = ((c_fitTimeMin - t) + imind) * double(c_nSamplesPerAdcClock); // starting point in sampled signal array
  int jmin = w; // starting point in sampled signal array
  w -= jmin;

  // use linear interpolation between samples. Since signal samples
  // are aligned with output samples only two weights are need to
  // calculate to fill output array
  const double w1 = amplitude * w, w0 = amplitude - w1;
  double sum = 0;
  for (int i = imin, j = jmin; i < imax; i++, j += c_nSamplesPerAdcClock) {
    double amp = w0 * m_signalShape[j] + w1 * m_signalShape[j + 1];
    signal[i] += amp;
    sum  += amp;
  }
  return sum * m_sumScale;
}
