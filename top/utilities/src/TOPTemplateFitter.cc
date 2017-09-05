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
#include <math.h>

#include <iostream>

using namespace Belle2;
using namespace Belle2::TOP;

using namespace std;

TOPTemplateFitter::TemplateParameters TOPTemplateFitter::s_templateParameters;
int TOPTemplateFitter::s_totalTemplateSamples = 64;
int TOPTemplateFitter::s_templateResolution = 4;
int TOPTemplateFitter::s_fineOffsetRange = TOPTemplateFitter::s_templateResolution * 4;
std::vector<double> TOPTemplateFitter::s_templateSamples;
bool TOPTemplateFitter::s_templateReInitialize = true;

TOPTemplateFitter::TOPTemplateFitter(const TOPRawWaveform& wf,
                                     const TOPSampleTimes& sampleTimes,
                                     const double averageRMS)
  : m_wf(wf), m_sampleTimes(sampleTimes), m_averageRMS(averageRMS)
{
  m_chisq = -1.;
  if (s_templateReInitialize) {
    InitializeTemplateFit();
  }
}

void TOPTemplateFitter::setTemplateParameters(const TemplateParameters& params)
{
  s_templateParameters = params;
  s_templateReInitialize = true;
}

void TOPTemplateFitter::setTemplateSamples(int nSamples)
{
  s_totalTemplateSamples = nSamples;
  s_templateReInitialize = true;

}

void TOPTemplateFitter::setTemplateResolution(int resolution)
{
  s_templateResolution = resolution;
  s_fineOffsetRange = resolution * 4;
  s_templateReInitialize = true;
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
    a = pow(s_templateParameters.n / absAlpha, s_templateParameters.n) * exp(-0.5 * absAlpha * absAlpha);
    b = s_templateParameters.n / absAlpha - absAlpha;
    value = a / pow(b - t, s_templateParameters.n) * s_templateParameters.amplitude;
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
  return value / integrationSteps;
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
  s_templateReInitialize = false;
}

void TOPTemplateFitter::performTemplateFit(const double risingEdgeStart,
                                           const double fitRange)
{
  if (s_templateReInitialize) {
    InitializeTemplateFit();
  }//do this here to make sure nobody changed template settings? -> thread safety?

  //access time base correction and calculate correction to waveform sample number
  vector<short> pedestals(m_wf.getSize(), m_averageRMS);//for now assume constant pedestals for TOP
  vector<float> timingCorrection(m_wf.getSize(),
                                 0.);//timing correction, should be relative to each sample in in units of samples -> pick correct template sample
  //perform template fit
  PerformTemplateFitMinimize(m_wf.getWaveform(), pedestals, timingCorrection, risingEdgeStart, fitRange);
}

double TOPTemplateFitter::GetTemplateRisingEdge()
{
  return s_templateParameters.mean - sqrt(2 * log(2)) * s_templateParameters.sigma;
}

void TOPTemplateFitter::PerformTemplateFitMinimize(const std::vector<short>& samples, const std::vector<short>& pedestals,
                                                   const std::vector<float>& timingCorrection, const double risingEdgeCFD, const double fitRange)
{
  if (samples.size() != pedestals.size() || samples.size() != timingCorrection.size()) {
    B2FATAL("Size of sample, pedestal and timing correction vectors has to be equal!");
  }

  MinimizationSums sums;
  FitResult result;
  m_chisq = 1e6;
  int initialOffset = risingEdgeCFD - GetTemplateRisingEdge();
  //offset search loop
  for (int fineOffset = -s_fineOffsetRange; fineOffset < s_fineOffsetRange; ++fineOffset) {
    sums.clear();
    int totalTemplateOffset = initialOffset * s_templateResolution + fineOffset;
    for (int signalSampleIndex = risingEdgeCFD - fitRange; signalSampleIndex < risingEdgeCFD + fitRange; ++signalSampleIndex) {
      if (signalSampleIndex < 0 || signalSampleIndex > (int)samples.size() - 1) continue;
      int templateIndex = signalSampleIndex * s_templateResolution - totalTemplateOffset + timingCorrection[signalSampleIndex];
      if (templateIndex < 0 || templateIndex > (int) s_templateSamples.size() - 1) continue;
      double weight = 1. / (pedestals[signalSampleIndex] * pedestals[signalSampleIndex]);
      double Sx = weight * s_templateSamples[templateIndex];
      double Sy = weight * samples[signalSampleIndex];
      sums.S1 += weight;
      sums.Sx += Sx;
      sums.Sy += Sy;
      sums.Sxx += Sx * s_templateSamples[templateIndex];
      sums.Sxy += Sy * s_templateSamples[templateIndex];
      sums.Syy += Sy * samples[signalSampleIndex];
    }
    double chisq = ComputeMinimizedParametersAndChisq(sums, result);
    if (chisq < m_chisq) { //save minimal chisq result
      m_chisq = chisq;
      m_result = result;
      m_result.risingEdge = totalTemplateOffset;
    }
  }
  //template offset back to sample space and scale amplitude
  m_result.risingEdge = m_result.risingEdge / s_templateResolution + GetTemplateRisingEdge();
  m_result.amplitude *= s_templateParameters.amplitude;
  m_result.amplitudeError *= s_templateParameters.amplitude;
}

double TOPTemplateFitter::ComputeMinimizedParametersAndChisq(const MinimizationSums& sums, FitResult& result)
{
  const double determinant = sums.S1 * sums.Sxx - sums.Sx * sums.Sx;
  result.amplitude = (-sums.Sx * sums.Sy + sums.S1 * sums.Sxy) / determinant;
  result.backgroundOffset = (sums.Sxx * sums.Sy - sums.Sx * sums.Sxy) / determinant;
  result.amplitudeError = sums.S1 / determinant;
  result.backgroundOffsetError = sums.Sxx / determinant;
  return sums.Syy - result.backgroundOffset * sums.Sy - result.amplitude * sums.Sxy;
}
