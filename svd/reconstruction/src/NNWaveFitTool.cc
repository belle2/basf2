/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/NNWaveFitTool.h>
#include <memory>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

tuple<double, double> NNWaveFitTool::getTimeShift(const nnFitterBinData& p)
{
  // calculate time estimate and error
  double timeShift = inner_product(p.begin(), p.end(), m_binCenters.begin(), 0.0);
  // use altBinData storage to sum squared residuals
  transform(m_binCenters.begin(), m_binCenters.end(), m_altBinData.begin(),
            [timeShift](double t)->double { return (t - timeShift) * (t - timeShift);});
  double timeShiftError = sqrt(inner_product(p.begin(), p.end(), m_altBinData.begin(), 0.0));
  return make_tuple(timeShift, timeShiftError);
}

tuple<double, double, double> NNWaveFitTool::getAmplitudeChi2(const apvSamples& samples,
    double timeShift, double tau)
{
  // Amplitude
  auto tw = m_waveGenerator(timeShift, tau);
  double waveNorm = inner_product(
                      tw.begin(), tw.end(), tw.begin(), 0.0);
  double amplitude = 0.0;
  double amplitudeError = 100.0;
  if (waveNorm > 0.0) {
    amplitude = inner_product(samples.begin(), samples.end(),
                              tw.begin(), 0.0) / waveNorm;
    amplitudeError = 1.0 / sqrt(waveNorm);
  }
  // Chi2
  transform(samples.begin(), samples.end(), tw.begin(), m_altSamples.begin(),
            [amplitude](double s, double w)->double { return s - w* amplitude;});
  size_t ndf = accumulate(samples.begin(), samples.end(), size_t(0),
                          [](size_t sum, double x)->size_t { return ((x > 3) ? sum + 1 : sum); }
                         ) - 2;
  double chi2 = sqrt(1.0 / ndf * inner_product(m_altSamples.begin(), m_altSamples.end(),
                                               m_altSamples.begin(), 0.0));
  return make_tuple(amplitude, amplitudeError, chi2);
}

void NNWaveFitTool::shiftInTime(nnFitterBins& p, double timeShift)
{
  // Calculate  at bin boundaries shifted by timeShift, new p's are differences.
  EmpiricalDistributionFunction edf(p, m_bins);
  auto ibin = m_bins.begin();
  double lowEdf = edf(-timeShift + *ibin++);
  for (auto& prob : p) {
    double highEdf = edf(-timeShift + *ibin++);
    prob = highEdf - lowEdf;
    lowEdf = highEdf;
  }
  normalize(p);
}

shared_ptr<nnFitterBinData> NNWaveFitTool::pFromInterval(double left, double right)
{
  auto uniCdf = [left, right](double x)->double {
    if (x < left) return 0.0;
    if (x > right) return 1.0;
    return (x - left) / (right - left);
  };
  auto result = shared_ptr<nnFitterBinData>(new nnFitterBinData(m_binCenters.size()));
  for (size_t i = 1; i < m_bins.size(); ++i)(*result)[i - 1] = uniCdf(m_bins[i] - m_bins[i - 1]);
  return result;
}

double NNWaveFitTool::pLessThan(nnFitterBinData p1, nnFitterBinData p2)
{
  // The formula is integral(P1(x)F2(x)dx.
  // We do sum(F1[i]P[i]
  EmpiricalDistributionFunction edf1(p1, m_bins);
  double result = 0;
  for (size_t i2 = 1; i2 < m_binCenters.size(); ++i2)
    result += edf1(m_binCenters[i2 - 1]) * p2[i2];
  return result;
}
