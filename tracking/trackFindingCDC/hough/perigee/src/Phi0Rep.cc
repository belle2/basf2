/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Phi0BinsSpec::Phi0BinsSpec(long nBins, int nOverlap, int nWidth)
  : m_nBins(nBins)
  , m_nOverlap(nOverlap)
  , m_nWidth(nWidth)
{
  B2ASSERT("Overlap must be smaller than the width.", m_nWidth > m_nOverlap);
}

DiscretePhi0::Array Phi0BinsSpec::constructArray() const
{
  const long nPositions = getNPositions();
  const double overlap = getOverlap();
  // Adjust the angle bounds such that overlap occures at the wrap around as well
  const double lowerBound = -M_PI - overlap / 2;
  const double upperBound = +M_PI + overlap / 2;
  return linspace<Vector2D>(lowerBound, upperBound, nPositions, &(Vector2D::Phi));
}

long Phi0BinsSpec::getNPositions() const
{
  const long nPositions = (m_nWidth - m_nOverlap) * m_nBins + m_nOverlap + 1;
  return nPositions;
}

double Phi0BinsSpec::getBinWidth() const
{
  return 2.0 * M_PI / m_nBins + getOverlap();
}

double Phi0BinsSpec::getOverlap() const
{
  // Overlap carefully calculated such that the overlap also occures at
  // the wrap around near M_PI to -M_PI with the same with without spoiling the
  // linearity of the binning.
  const double overlap = 2.0 * M_PI * m_nOverlap / (m_nBins * (m_nWidth - m_nOverlap));
  return overlap;
}
