/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Weber                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/utilities/TOPTemplateFitter.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace Belle2::TOP;

TemplateParameters TOPTemplateFitter::s_templateParameters;
int TOPTemplateFitter::s_totalTemplateSamples = 64;
int TOPTemplateFitter::s_templateResolution = 4;
std::vector<double> TOPTemplateFitter::s_templateSamples;

TOPTemplateFitter::TOPTemplateFitter(const TOPRawWaveform& wf,
                                     const TOPSampleTimes& sampleTimes,
                                     const double averageRMS)
  : m_wf(wf), m_sampleTimes(sampleTimes), m_averageRMS(averageRMS)
{
  m_chisq = -1.;
  InitializeTemplateFit();
}

void TOPTemplateFitter::SetTemplateParameters(const TemplateParameters& params)
{
  s_templateParameters = params;
  InitializeTemplateFit();
}

void TOPTemplateFitter::SetTemplateSamples(int nSamples)
{
  s_totalTemplateSamples = nSamples;
  InitializeTemplateFit();
}

void TOPTemplateFitter::SetTemplateResolution(int resolution)
{
  s_templateResolution = resolution;
  InitializeTemplateFit();
}

double TOPTemplateFitter::Crystalball(const double x)
{
  double value, t, absAlpha;
  double a, b;

  //crystalball function from roofit
  t = (x - s_templateParameters.mean) / s_templateParameters.sigma;
  if (s_templateParameters.alpha < 0) t = -t;
  if (s_templateParameters.alpha > 0) absAlpha = s_templateParameters.alpha;
  else absAlpha = -s_templateParameters.alpha;

  if (t >= -absAlpha) {
    value = exp(-0.5 * t * t) * s_templateParameters.amplitude;
  } else {
    a = powf(s_templateParameters.n / absAlpha, s_templateParameters.n) * exp(-0.5 * absAlpha * absAlpha);
    b = s_templateParameters.n / absAlpha - absAlpha;
    value = a / powf(b - t, s_templateParameters.n) * s_templateParameters.amplitude;
  }
  return value;
}

double TOPTemplateFitter::CalculateTemplate(const double x)
{
  //mimic integration of IRSX ASIC by integrating one bin around x value
  const int integrationSteps = 32;
  const double stepSize = 1. / integrationSteps;
  double value = 0;
  for (int i = 0; i < integrationSteps; ++i) value += Crystalball(x - 0.5 + stepSize * i);
  return value * stepSize;
}

void TOPTemplateFitter::InitializeTemplateFit()
{
  s_templateSamples.clear();
  if (s_templateParameters.mean < 0 || s_templateParameters.mean > s_totalTemplateSamples) {
    B2FATAL("mean of template function invalid!");
  }
  for (double x = 0; x < s_totalTemplateSamples; x += 1. / s_templateResolution) {
    s_templateSamples.push_back(CalculateTemplate(x));
  }
}

void TOPTemplateFitter::PerformTemplateFit(const double risingEdgeStart,
                                           const double fitRange)
{
  //access time base correction and calculate correction to waveform sample number

  //perform template fit
}

/**
@brief  Compute the minimized parameters and chi square value
@param minimization sums for chisq calculation
@param minimized parameters
@return chi square
*/
inline double ComputeMinimizedParametersAndChisq(const MinimizationSums& sums, FitResult& result)
{
  const double determinant = sums.S1 * sums.Sxx - sums.Sx * sums.Sx;
  result.amplitudeScaling = (-sums.Sx * sums.Sy + sums.S1 * sums.Sxy) / determinant;
  result.backgroundOffset = (sums.Sxx * sums.Sy - sums.Sx * sums.Sxy) / determinant;
  result.amplitudeScalingErr = sums.S1 / determinant;
  result.backgroundOffsetError = sums.Sxx / determinant;
  return sums.Syy - result.backgroundOffset * sums.Sy - result.amplitudeScaling * sums.Sxy;
}
