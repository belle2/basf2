/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/hough/perigee/DiscretePhi0.h>

using namespace Belle2;
using namespace TrackFindingCDC;


Phi0BinsSpec::Phi0BinsSpec(size_t nBins, size_t nOverlap, size_t nWidth) :
  m_nBins(nBins),
  m_nOverlap(nOverlap),
  m_nWidth(nWidth)
{
  B2ASSERT("Overlap must be smaller than the width.",
           m_nWidth > m_nOverlap);
}

DiscretePhi0Array Phi0BinsSpec::constructArray() const
{
  const size_t nPositions = getNPositions();
  const double overlap = getOverlap();
  // Adjust the angle bounds such that overlap occures at the wrap around as well
  const double lowerBound = -PI - overlap / 2;
  const double upperBound = +PI + overlap / 2;
  return DiscretePhi0Array(lowerBound, upperBound, nPositions);
}

size_t Phi0BinsSpec::getNPositions() const
{
  const size_t nPositions = (m_nWidth - m_nOverlap) * m_nBins + m_nOverlap + 1;
  return nPositions;
}

double Phi0BinsSpec::getBinWidth() const
{
  return 2.0 * PI / m_nBins + getOverlap();
}

double Phi0BinsSpec::getOverlap() const
{
  // Overlap carefully calculated such that the overlap also occures at
  // the wrap around near PI to -PI with the same with without spoiling the
  // linearity of the binning.
  const double overlap = 2.0 * PI * m_nOverlap / (m_nBins * (m_nWidth - m_nOverlap));
  return overlap;
}
