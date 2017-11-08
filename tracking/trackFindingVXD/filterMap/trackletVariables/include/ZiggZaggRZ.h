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
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
#include <framework/geometry/B2Vector3.h>
#include <boost/math/special_functions/sign.hpp>
#include <math.h>

#include <vector>
#include <algorithm>


#define ZIGGZAGGRZ_NAME ZiggZaggRZ

namespace Belle2 {

  /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging).
   *
   * ATTENTION: in contrast to the Two- Three- and FourHit-filters, the TrackletFilters do _NOT_ expect Hits, but a Container with pointers to Hits (e.g. Vector of hits or a SpacePointTrackCandidate, These filters therefore apply to arbitrary numbers of hits in that container.
   * */
  template <typename PointType, typename PointContainerType >
  class ZIGGZAGGRZ_NAME : public SelectionVariable< PointContainerType , 0, int > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(ZIGGZAGGRZ_NAME);


    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging) */
    static int value(const PointContainerType& hitContainer)
    {
      const unsigned nHits = hitContainer.size();
      if (nHits < 4) return 1;

      typedef SelVarHelper<PointType, double> Helper;
      using boost::math::sign;

      std::vector<B2Vector3D> vecRZ;
      vecRZ.reserve(nHits);
      for (const auto* hit : hitContainer) { // collect RZ-Vrsions of the hits:
        vecRZ.push_back(B2Vector3D(Helper::calcPerp(*hit) , hit->Z(), 0.));
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
  };

}
