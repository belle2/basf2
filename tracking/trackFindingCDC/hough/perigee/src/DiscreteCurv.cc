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

DiscreteCurvArray
DiscreteCurvArray::forCurvBinsWithOverlap(float minCurv,
                                          float maxCurv,
                                          size_t nBins,
                                          size_t nWidth,
                                          size_t nOverlap)
{
  const size_t nPositions = (nWidth - nOverlap) * nBins + nOverlap + 1;
  const float lowerBound = minCurv;
  const float upperBound = maxCurv;
  return DiscreteCurvArray(lowerBound, upperBound, nPositions);
}



DiscreteCurvArray
DiscreteCurvArray::forPositiveCurvBinsWithOverlap(float maxCurv,
                                                  size_t nBins,
                                                  size_t nWidth,
                                                  size_t nOverlap)
{
  B2ASSERT((maxCurv > 0), "Maximum curvature values must be positive for positive curvature searches.");
  B2ASSERT((nWidth > nOverlap), "Overlap must be smaller than the width.");

  const size_t nPositions = (nWidth - nOverlap) * nBins + nOverlap + 1;

  // Determining the lower bound such that the first bin is symmetric around zero
  // This prevents some cut of effects if the hit happens to lean to
  // the negative curvature spaces.
  const double overlapWidthRatio = static_cast<double>(nOverlap) / nWidth;
  const double width = maxCurv / (nBins * (1 - overlapWidthRatio) + overlapWidthRatio - 0.5);
  const double lowerBound = -width / 2;
  const double upperBound = maxCurv;
  return DiscreteCurvArray(lowerBound, upperBound, nPositions);
}
