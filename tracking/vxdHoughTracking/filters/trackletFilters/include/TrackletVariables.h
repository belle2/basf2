/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/pathFilters/ThreeHitVariables.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/geometry/B2Vector3.>

#include <cmath>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane,
    /// returns number of charge-signs found (if != 1, then the given sptc is ziggZagging) */
    int calculateZiggZaggRZ(const SpacePointTrackCand& sptc)
    {
      const unsigned nHits = sptc.size();
      if (nHits < 4) return 1;

      using boost::math::sign;

      std::vector<B2Vector3D> vecRZ;
      vecRZ.reserve(nHits);
      for (const auto* hit : sptc) { // collect RZ-Vrsions of the hits:
        vecRZ.push_back(B2Vector3D(hit->Perp(), hit->Z(), 0.));
      }

      std::vector<int> chargeSigns;
      chargeSigns.reserve(nHits - 2);
      for (unsigned i = 0; i < nHits - 2; ++i) {
        int signVal = sign((vecRZ.at(i + 1) - vecRZ.at(i + 2)).Orthogonal() * (vecRZ.at(i) - vecRZ.at(i + 1)));
        chargeSigns.push_back(signVal);
      }

      std::sort(chargeSigns.begin(), chargeSigns.end());
      auto newEnd = std::unique(chargeSigns.begin(), chargeSigns.end());

      return std::distance(chargeSigns.begin(), newEnd);
    } // return unit: none


    /// checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane,
    /// returns number of charge-signs found (if != 1, then the given sptc is ziggZagging) */
    static int calculateZiggZaggXY(const SpacePointTrackCand& sptc)
    {
      if (sptc.size() < 4) return 1;

      std::vector<int> chargeSigns;
      chargeSigns.reserve(sptc.size() - 2);

      auto iterPos = sptc.begin();
      auto stopPos = sptc.end() - 2;

      ThreeHitVariables threeHitVariables;

      for (; iterPos < stopPos; ++iterPos) {
        int signVal = threeHitVariables.getCurvatureSign(**iterPos, **(iterPos + 1), **(iterPos + 2));
        chargeSigns.push_back(signVal);
      }

      std::sort(chargeSigns.begin(), chargeSigns.end());
      auto newEnd = std::unique(chargeSigns.begin(), chargeSigns.end());

      return std::distance(chargeSigns.begin(), newEnd);
    } // return unit: none

  }
}
