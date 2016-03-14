/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/FilterTools/SelectionVariable.h>
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelper.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#include <vector>
#include <algorithm>

namespace Belle2 {

  /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging). This functions uses the sigma to consider also approximately straight tracks as not zigg-zagging.
   *
   * ATTENTION: in contrast to the Two- Three- and FourHit-filters, the TrackletFilters do _NOT_ expect Hits, but a Container with pointers to Hits (e.g. Vector of hits or a SpacePointTrackCandidate, These filters therefore apply to arbitrary numbers of hits in that container.
   * */
  template <typename PointType, typename PointContainerType >
  class ZiggZaggXYWithSigma : public SelectionVariable< PointContainerType , int > {
  public:

    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging). This functions uses the sigma to consider also approximately straight tracks as not zigg-zagging */
    static int value(const PointContainerType& hitContainer)
    {
      if (hitContainer.size() < 4) return 1;

      typedef SelVarHelper<PointType, double> Helper;

      std::vector<int> chargeSigns;
      chargeSigns.reserve(hitContainer.size() - 2);

      auto iterPos = hitContainer.begin();
      auto stopPos = hitContainer.end() - 2;

      for (; iterPos < stopPos; ++iterPos) {
        int signVal = Helper::calcSign(**iterPos, **(iterPos + 1), **(iterPos + 2), (*iterPos)->getPositionError(),
                                       (*(iterPos + 1))->getPositionError(), (*(iterPos + 2))->getPositionError());
        chargeSigns.push_back(signVal);
      }

      std::sort(chargeSigns.begin(), chargeSigns.end());
      auto endAfterRemove = std::remove(chargeSigns.begin(), chargeSigns.end(), 0);
      auto finalEnd = std::unique(chargeSigns.begin(), endAfterRemove);

      return std::distance(chargeSigns.begin(), finalEnd);
    } // return unit: none
  };

}
