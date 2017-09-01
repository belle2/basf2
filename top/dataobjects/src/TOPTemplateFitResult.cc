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

  TOPTemplateFitResult::TOPTemplateFitResult() {}

  TOPTemplateFitResult::TOPTemplateFitResult(unsigned short risingEdge, short backgroundOffset,
                                             short amplitude, unsigned short chisquare)
  {
    m_risingEdgeRaw = risingEdge;
    m_risingEdge = risingEdgeShortToRisingEdgeDouble(risingEdge);
    m_backgroundOffset = backgroundOffset;
    m_amplitude = amplitude;
    m_chisquare = chisquare;
  }

  void TOPTemplateFitResult::setRisingEdge(unsigned short risingEdge)
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

  double TOPTemplateFitResult::getMean() const
  {
    //center of template is gaussian function -> there is a relation between rising edge
    //position and template mean
    return m_risingEdge + sqrt(2 * logf(2)) * m_templateSigma;
  }

  double TOPTemplateFitResult::getTemplateFunctionValue(double x) const
  {
    return crystalball(x) + m_backgroundOffset;
  }

  double TOPTemplateFitResult::crystalball(double x) const
  {
    double absAlpha;
    double value;
    double t = (x - getMean()) / m_templateSigma;
    if (m_templateAlpha < 0) t = -t;
    if (m_templateAlpha > 0) absAlpha = m_templateAlpha;
    else absAlpha = -m_templateAlpha;

    if (t >= -absAlpha) {
      value = exp(-0.5 * t * t) * m_amplitude;
    } else {
      double a = powf(m_templateN / absAlpha, m_templateN) * exp(-0.5 * absAlpha * absAlpha);
      double b = m_templateN / absAlpha - absAlpha;
      value = a / powf(b - t, m_templateN) * m_amplitude;
    }
    return value;
  }

} // end Belle2 namespace
