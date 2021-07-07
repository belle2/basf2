/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/SignCurvatureXYError.h>

#include <vector>
#include <algorithm>

#define ZIGGZAGGXYWITHSIGMA_NAME ZiggZaggXYWithSigma

namespace Belle2 {

  /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging). This functions uses the sigma to consider also approximately straight tracks as not zigg-zagging.
   *
   * ATTENTION: in contrast to the Two- Three- and FourHit-filters, the TrackletFilters do _NOT_ expect Hits, but a Container with pointers to Hits (e.g. Vector of hits or a SpacePointTrackCandidate, These filters therefore apply to arbitrary numbers of hits in that container.
   * */
  template <typename PointType, typename PointContainerType >
  class ZIGGZAGGXYWITHSIGMA_NAME : public SelectionVariable< PointContainerType , 0, int > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(ZIGGZAGGXYWITHSIGMA_NAME);


    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging). This functions uses the sigma to consider also approximately straight tracks as not zigg-zagging */
    static int value(const PointContainerType& hitContainer)
    {
      if (hitContainer.size() < 4) return 1;


      std::vector<int> chargeSigns;
      chargeSigns.reserve(hitContainer.size() - 2);

      auto iterPos = hitContainer.begin();
      auto stopPos = hitContainer.end() - 2;

      for (; iterPos < stopPos; ++iterPos) {
        int signVal = SignCurvatureXYError<PointType>::value(**iterPos, **(iterPos + 1), **(iterPos + 2));
        chargeSigns.push_back(signVal);
      }

      std::sort(chargeSigns.begin(), chargeSigns.end());
      auto endAfterRemove = std::remove(chargeSigns.begin(), chargeSigns.end(), 0);
      auto finalEnd = std::unique(chargeSigns.begin(), endAfterRemove);

      return std::distance(chargeSigns.begin(), finalEnd);
    } // return unit: none
  };

}
