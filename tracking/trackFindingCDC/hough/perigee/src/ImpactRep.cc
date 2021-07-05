/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/perigee/ImpactRep.h>
#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ImpactBinsSpec::ImpactBinsSpec(double lowerBound, double upperBound, long nBins, int nOverlap, int nWidth)
  : m_lowerBound(lowerBound)
  , m_upperBound(upperBound)
  , m_nBins(nBins)
  , m_nOverlap(nOverlap)
  , m_nWidth(nWidth)
{
  B2ASSERT("Overlap must be smaller than the width.", m_nWidth > m_nOverlap);
  B2ASSERT("Upper impactature bound must be higher than the lower bound.", m_upperBound > m_lowerBound);
}

DiscreteImpact::Array ImpactBinsSpec::constructArray() const
{
  const long nPositions = getNPositions();
  return linspace<float>(m_lowerBound, m_upperBound, nPositions);
}

long ImpactBinsSpec::getNPositions() const
{
  const long nPositions = (m_nWidth - m_nOverlap) * m_nBins + m_nOverlap + 1;
  return nPositions;
}

double ImpactBinsSpec::getBinWidth() const
{
  const double overlapWidthRatio = static_cast<double>(m_nOverlap) / m_nWidth;
  const double width = (m_upperBound - m_lowerBound) / (m_nBins * (1 - overlapWidthRatio) + overlapWidthRatio);
  return width;
}

double ImpactBinsSpec::getOverlap() const
{
  return getBinWidth() * m_nOverlap / m_nWidth;
}
