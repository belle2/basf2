/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/perigee/DiscreteCurv.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CurvBinsSpec::CurvBinsSpec(double lowerBound,
                           double upperBound,
                           size_t nBins,
                           size_t nOverlap,
                           size_t nWidth):
  m_lowerBound(lowerBound),
  m_upperBound(upperBound),
  m_nBins(nBins),
  m_nOverlap(nOverlap),
  m_nWidth(nWidth)
{
  B2ASSERT("Overlap must be smaller than the width.",
           m_nWidth > m_nOverlap);

  B2ASSERT("Upper curvature bound must be higher than the lower bound.",
           m_upperBound > m_lowerBound);
}


DiscreteCurv::Array CurvBinsSpec::constructArray() const
{
  const size_t nPositions = getNPositions();
  if (m_lowerBound == 0) {
    // Determining the lower bound such that the first bin is symmetric around zero
    // This prevents some cut of effects if the hit happens to lean to
    // the negative curvature spaces.
    const double width = getBinWidth();
    const double expandedLowerBound = -width / 2;
    return linspace<float>(expandedLowerBound, m_upperBound, nPositions);
  } else {
    return linspace<float>(m_lowerBound, m_upperBound, nPositions);
  }
}

size_t CurvBinsSpec::getNPositions() const
{
  assert(m_nWidth > m_nOverlap);
  const size_t nPositions = (m_nWidth - m_nOverlap) * m_nBins + m_nOverlap + 1;
  return nPositions;
}


double CurvBinsSpec::getBinWidth() const
{
  const double overlapWidthRatio = static_cast<double>(m_nOverlap) / m_nWidth;
  if (m_lowerBound == 0) {
    // Determining the lower bound such that the first bin is symmetric around zero
    // This prevents some cut of effects if the hit happens to lean to
    // the negative curvature spaces.
    const double width = m_upperBound / (m_nBins * (1 - overlapWidthRatio) + overlapWidthRatio - 0.5);
    return width;
  } else {
    const double width = (m_upperBound - m_lowerBound) / (m_nBins * (1 - overlapWidthRatio) + overlapWidthRatio);
    return width;
  }
}

double CurvBinsSpec::getOverlap() const
{
  return getBinWidth() * m_nOverlap / m_nWidth;
}
