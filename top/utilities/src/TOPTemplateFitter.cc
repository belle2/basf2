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
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>

#include <algorithm>
#include <math.h>

using namespace Belle2;
using namespace Belle2::TOP;

using namespace std;

TOPTemplateFitter::TemplateParameters TOPTemplateFitter::s_templateParameters;
int TOPTemplateFitter::s_totalTemplateSamples = 64;
int TOPTemplateFitter::s_templateResolution = 4;
int TOPTemplateFitter::s_fineOffsetRange = TOPTemplateFitter::s_templateResolution * 5;
std::vector<double> TOPTemplateFitter::s_templateSamples;
bool TOPTemplateFitter::s_templateReInitialize = true;
bool TOPTemplateFitter::s_useParabola = true;

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
  s_fineOffsetRange = resolution * 5;
  s_templateReInitialize = true;
}

void TOPTemplateFitter::InitializeTemplateFit()
{
  s_templateSamples.clear();
  if (s_templateParameters.risingEdge < 0 || s_templateParameters.risingEdge > s_totalTemplateSamples) {
    B2FATAL("rising edge of template function invalid!");
  }

  const auto* geo = TOPGeometryPar::Instance()->getGeometry();
  const auto& signalShape = geo->getSignalShape();
  const auto& tdc = geo->getNominalTDC();
  const double dt = tdc.getSampleWidth();

  for (double x = 0; x < s_totalTemplateSamples; x += 1. / s_templateResolution) {
    s_templateSamples.push_back(s_templateParameters.amplitude * signalShape.getValue((x - s_templateParameters.risingEdge)*dt));
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
  m_chisq_vec.clear();
  m_chisq = 1e6;
  PerformTemplateFitMinimize(m_wf.getWaveform(), pedestals, timingCorrection, risingEdgeStart, fitRange);

  //use paraboic shape of chi^2 distribution to improve fit result
  if (s_useParabola) {
    auto it = std::min_element(std::begin(m_chisq_vec), std::end(m_chisq_vec));
    int minidx = std::distance(std::begin(m_chisq_vec), it);
    if (minidx > 0 && minidx < (int)m_chisq_vec.size() - 1) {
      Point vertex;
      CalculateParabolaVertex({ -1, m_chisq_vec[minidx - 1]}, {0, m_chisq_vec[minidx]}, {1, m_chisq_vec[minidx + 1]}, vertex);
      m_chisq = vertex.second;
      m_result.risingEdge += vertex.first / s_templateResolution;
    }
  }
}

void TOPTemplateFitter::CalculateParabolaVertex(const Point& p1, const Point& p2, const Point& p3, Point& vertex)
{
  double denom = (p1.first - p2.first) * (p1.first - p3.first) * (p2.first - p3.first);
  double a     = (p3.first * (p2.second - p1.second) + p2.first * (p1.second - p3.second) + p1.first *
                  (p3.second - p2.second)) / denom;
  double b     = (p3.first * p3.first * (p1.second - p2.second) + p2.first * p2.first * (p3.second - p1.second)
                  + p1.first * p1.first * (p2.second - p3.second)) / denom;
  double c     = (p2.first * p3.first * (p2.first - p3.first) * p1.second + p3.first * p1.first * (p3.first - p1.first) * p2.second
                  + p1.first * p2.first * (p1.first - p2.first) * p3.second) / denom;

  vertex.first = -b / (2 * a);
  vertex.second = c - b * b / (4 * a);
}

void TOPTemplateFitter::PerformTemplateFitMinimize(const std::vector<short>& samples, const std::vector<short>& pedestals,
                                                   const std::vector<float>& timingCorrection, const double risingEdgeCFD, const double fitRange)
{
  if (samples.size() != pedestals.size() || samples.size() != timingCorrection.size()) {
    B2FATAL("Size of sample, pedestal and timing correction vectors has to be equal!");
  }


  MinimizationSums sums;
  FitResult result;
  int initialOffset = risingEdgeCFD - s_templateParameters.risingEdge;
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
    m_chisq_vec.push_back(chisq);
    if (chisq < m_chisq) { //save minimal chisq result
      m_chisq = chisq;
      m_result = result;
      m_result.risingEdge = totalTemplateOffset;
    }
  }
  //template offset back to sample space and scale amplitude
  m_result.risingEdge = m_result.risingEdge / s_templateResolution + s_templateParameters.risingEdge;
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
