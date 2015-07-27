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
#include <tracking/vectorTools/B2Vector3.h>
#include <boost/math/special_functions/sign.hpp>
#include <math.h>

#include <vector>
#include <algorithm>

namespace Belle2 {

  /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging).
   *
   * ATTENTION: in contrast to the Two- Three- and FourHit-filters, the TrackletFilters do _NOT_ expect Hits, but a Container with pointers to Hits (e.g. Vector of hits or a SpacePointTrackCandidate, These filters therefore apply to arbitrary numbers of hits in that container.
   * */
  template <typename PointType, typename PointContainerType >
  class ZiggZaggRZ : public SelectionVariable< PointContainerType , int > {
  public:

    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns number of charge-signs found (if != 1, then the given hitContainer is ziggZagging) */
    static int value(const PointContainerType& hitContainer)
    {
      const unsigned nHits = hitContainer.size();
      if (nHits < 4) return 1;

      typedef SelVarHelper<PointType, float> Helper;
      using boost::math::sign;

      std::vector<B2Vector3F> vecRZ;
      vecRZ.reserve(nHits);
      for (const auto* hit : hitContainer) { // collect RZ-Vrsions of the hits:
        vecRZ.push_back(B2Vector3F(Helper::calcPerp(*hit) , hit->Z(), 0.));
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
/*
using boost::math::sign;
B2Vector3<DataType> ba = doAMinusB(a, b); ba.SetZ(0.);
B2Vector3<DataType> bc = doAMinusB(b, c); bc.SetZ(0.);
return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba

bool TrackletFilters::ziggZaggRZ()
{
  if (m_hits == NULL) B2FATAL(" TrackletFilters::ziggZaggRZ: hits not set, therefore no calculation possible - please check that!")
  list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  vector<TVector3> rzHits;
  TVector3 currentVector;
  for (PositionInfo* aHit : *m_hits) {
  currentVector.SetXYZ(aHit->hitPosition.Perp(), aHit->hitPosition[1], 0.);
  rzHits.push_back(currentVector);
  }
  for (int i = 0; i < m_numHits - 2; ++i) {
  int signValue = m_3hitFilterBox.calcSign(rzHits.at(i), rzHits.at(i + 1), rzHits.at(i + 2));
  chargeSigns.push_back(signValue);
  }
  chargeSigns.sort();
  chargeSigns.unique();
  if (int(chargeSigns.size()) != 1) {
  isZiggZagging = true;
  }
  return isZiggZagging;
}
*/
