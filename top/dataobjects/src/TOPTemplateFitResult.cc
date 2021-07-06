/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dataobjects/TOPTemplateFitResult.h>
#include <math.h>

using namespace std;

namespace Belle2 {

  TOPTemplateFitResult::TOPTemplateFitResult() {}

  TOPTemplateFitResult::TOPTemplateFitResult(short risingEdge, short backgroundOffset,
                                             short amplitude, short chisquare)
  {
    m_risingEdgeRaw = risingEdge;
    m_risingEdge = risingEdgeShortToRisingEdgeDouble(risingEdge);
    m_backgroundOffset = backgroundOffset;
    m_amplitude = amplitude;
    m_chisquare = chisquare;
    m_risingEdgeTime = -1.;
  }

  TOPTemplateFitResult::TOPTemplateFitResult(double risingEdge, double risingEdgeTime, double backgroundOffset,
                                             double amplitude, double chisquare)
    : m_risingEdge(risingEdge), m_risingEdgeTime(risingEdgeTime), m_backgroundOffset(backgroundOffset), m_amplitude(amplitude),
      m_chisquare(chisquare)
  {
    m_risingEdgeRaw = -1;
  }

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

} // end Belle2 namespace
