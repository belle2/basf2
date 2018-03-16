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

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/SignCurvatureXY.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#include <vector>
#include <algorithm>

#define ZIGGZAGGXY_NAME ZiggZaggXY

namespace Belle2 {

  /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging).
   *
   * ATTENTION: in contrast to the Two- Three- and FourHit-filters, the TrackletFilters do _NOT_ expect Hits, but a Container with pointers to Hits (e.g. Vector of hits or a SpacePointTrackCandidate, These filters therefore apply to arbitrary numbers of hits in that container.
   * */
  template <typename PointType, typename PointContainerType >
  class ZIGGZAGGXY_NAME : public SelectionVariable< PointContainerType , 0, int > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(ZIGGZAGGXY_NAME);


    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging) */
    static int value(const PointContainerType& hitContainer)
    {
      if (hitContainer.size() < 4) return 1;

      std::vector<int> chargeSigns;
      chargeSigns.reserve(hitContainer.size() - 2);

      auto iterPos = hitContainer.begin();
      auto stopPos = hitContainer.end() - 2;

      for (; iterPos < stopPos; ++iterPos) {
        int signVal = SignCurvatureXY<PointType>::value(**iterPos, **(iterPos + 1), **(iterPos + 2));
        chargeSigns.push_back(signVal);
      }

      std::sort(chargeSigns.begin(), chargeSigns.end());
      auto newEnd = std::unique(chargeSigns.begin(), chargeSigns.end());

      return std::distance(chargeSigns.begin(), newEnd);
    } // return unit: none
  };

}
