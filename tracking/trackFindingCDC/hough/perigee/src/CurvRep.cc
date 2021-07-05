/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>
#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CurvBinsSpec::CurvBinsSpec(double lowerBound, double upperBound, long nBins, int nOverlap, int nWidth)
  : m_lowerBound(lowerBound)
  , m_upperBound(upperBound)
  , m_nBins(nBins)
  , m_nOverlap(nOverlap)
  , m_nWidth(nWidth)
{
  B2ASSERT("Overlap must be smaller than the width.", m_nWidth > m_nOverlap);
  B2ASSERT("Upper curvature bound must be higher than the lower bound.", m_upperBound > m_lowerBound);
}

DiscreteCurv::Array CurvBinsSpec::constructLinearArray() const
{
  const long nPositions = getNPositions();
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

DiscreteCurv::Array CurvBinsSpec::constructInvLinearArray() const
{
  const long nPositions = getNPositions();

  long nCurlingCurvs = 3 * nPositions / 10;

  std::vector<float> curlingCurvs = linspace<float>(1 / fabs(m_upperBound),
                                                    1 / fabs(m_lowerBound),
                                                    nCurlingCurvs + 1,
  [](float f) {return 1 / f;});

  std::sort(curlingCurvs.begin(), curlingCurvs.end());
  float minWidthCurling =  curlingCurvs[1] - curlingCurvs[0];


  std::vector<float> nonCurlingCurvs = linspace<float>(-fabs(m_lowerBound),
                                                       fabs(m_lowerBound),
                                                       nPositions - nCurlingCurvs);

  float widthNonCurling = nonCurlingCurvs[1] - nonCurlingCurvs[0];

  B2INFO("Min Width curling" <<  minWidthCurling);
  B2INFO("Width non curling" << widthNonCurling);
  assert(minWidthCurling > widthNonCurling);

  // ++ skips duplication of the identical curvature at m_lowerBound.
  nonCurlingCurvs.insert(nonCurlingCurvs.end(), ++(curlingCurvs.begin()), curlingCurvs.end());
  B2INFO(nonCurlingCurvs.size());
  B2INFO(nPositions);

  assert(nPositions == static_cast<long>(nonCurlingCurvs.size()));
  assert(std::is_sorted(nonCurlingCurvs.begin(), nonCurlingCurvs.end()));
  return nonCurlingCurvs;
}

DiscreteCurvWithArcLength2DCache::Array CurvBinsSpec::constructCacheArray() const
{
  std::vector<float> discreteCurvs = constructArray();
  std::vector<CurvWithArcLength2DCache> result;
  result.reserve(discreteCurvs.size());
  for (float curv : discreteCurvs) {
    result.emplace_back(curv);
  }
  return result;
}

long CurvBinsSpec::getNPositions() const
{
  assert(m_nWidth > m_nOverlap);
  const long nPositions = (m_nWidth - m_nOverlap) * m_nBins + m_nOverlap + 1;
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
