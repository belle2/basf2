/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Weber                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPTemplateFitResult.h>
#include <math.h>

using namespace std;

namespace Belle2 {

  float TOPTemplateFitResult::s_templateSigma = 1.81;
  float TOPTemplateFitResult::s_templateAlpha = -0.45;
  float TOPTemplateFitResult::s_templateN = 18.06;

  TOPTemplateFitResult::TOPTemplateFitResult() {}

  TOPTemplateFitResult::TOPTemplateFitResult(short risingEdge, short backgroundOffset,
                                             short amplitude, short chisquare)
  {
    m_risingEdgeRaw = risingEdge;
    m_risingEdge = risingEdgeShortToRisingEdgeDouble(risingEdge);
    m_backgroundOffset = backgroundOffset;
    m_amplitude = amplitude;
    m_chisquare = chisquare;
  }

  TOPTemplateFitResult::TOPTemplateFitResult(double risingEdge, double risingEdgeTime, double backgroundOffset,
                                             double amplitude, double chisquare)
    : m_risingEdge(risingEdge), m_risingEdgeTime(risingEdgeTime), m_backgroundOffset(backgroundOffset), m_amplitude(amplitude),
      m_chisquare(chisquare)
  {}

  void TOPTemplateFitResult::setRisingEdgeAndConvert(unsigned short risingEdge)
  {
    m_risingEdgeRaw = risingEdge;
    m_risingEdge = risingEdgeShortToRisingEdgeDouble(risingEdge);
  }

  double TOPTemplateFitResult::risingEdgeShortToRisingEdgeDouble(unsigned short risingEdgeS) const
  {
    double risingEdgeD = (risingEdgeS & 0xff00) >> 8;
    int mask = 0x80;
    for (int i = 0; i < 8; i++) {
      int b = (risingEdgeS & mask);
      if (b != 0) risingEdgeD += 1. / (pow(2, i + 1));
      mask = mask >> 1;
    }
    return risingEdgeD;
  }

  void TOPTemplateFitResult::setTemplateParameters(float sigma, float alpha, float n)
  {
    s_templateSigma = sigma;
    s_templateAlpha = alpha;
    s_templateN = n;
  }

  double TOPTemplateFitResult::getMean() const
  {
    //center of template is gaussian function -> there is a relation between rising edge
    //position and template mean
    return m_risingEdge + sqrt(2 * logf(2)) * s_templateSigma;
  }

  double TOPTemplateFitResult::getMeanTime() const
  {
    return m_risingEdgeTime + sqrt(2 * logf(2)) * s_templateSigma;
  }


  double TOPTemplateFitResult::getTemplateFunctionValue(double x) const
  {
    return crystalball(x) + m_backgroundOffset;
  }

  double TOPTemplateFitResult::crystalball(double x) const
  {
    double absAlpha;
    double value;
    double t = (x - getMean()) / s_templateSigma;
    if (s_templateAlpha < 0) t = -t;
    if (s_templateAlpha > 0) absAlpha = s_templateAlpha;
    else absAlpha = -s_templateAlpha;

    if (t >= -absAlpha) {
      value = exp(-0.5 * t * t) * m_amplitude;
    } else {
      double a = powf(s_templateN / absAlpha, s_templateN) * exp(-0.5 * absAlpha * absAlpha);
      double b = s_templateN / absAlpha - absAlpha;
      value = a / powf(b - t, s_templateN) * m_amplitude;
    }
    return value;
  }

} // end Belle2 namespace
