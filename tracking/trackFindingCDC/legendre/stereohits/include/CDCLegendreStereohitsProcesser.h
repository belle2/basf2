/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohit.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeNeighborFinder.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <TVector3.h>
#include <TVector2.h>

#include "boost/foreach.hpp"

#include <list>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <TMath.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;
    class TrackCandidate;
    class StereoHit;

    class StereohitsProcesser {
    public:

      StereohitsProcesser();

      ~StereohitsProcesser();


      /** Assign stereohits to the track with known polar angle */
      void assignStereohitsByAngle(TrackCandidate* cand, double theta, std::vector<TrackHit*>& stereohits, double Z0 = 0.);

      /** Return displacements of the stereohit against the track */
      std::pair<StereoHit, StereoHit> getDisplacements(TrackCandidate* cand, TrackHit* hit, int trackCharge = 0);

      /** Return displacement (inner or outer) of the stereohit against the track */
      StereoHit getDisplacement(TrackCandidate* cand, TrackHit* hit, int InnerOuter);

      /** Return displacement (inner or outer) of the stereohit against the track */
      void makeHistogramming(TrackCandidate* cand, std::vector<TrackHit*>& stereohits);

      /** Return position of the hit on the track expressed in rads */
      double getAlpha(TrackCandidate* cand, std::pair<double, double> pos);

      void assignStereohitsByAngleWithQuadtree(TrackCandidate* cand, double theta, std::vector<TrackHit*>& stereohits, double Z0 = 0.);

      void MaxFastHoughStereofinding(
        std::vector<TrackHit*>& hitsToAdd,
        std::vector<std::pair<StereoHit, StereoHit>>& hits,
        const int level,
        const double lambda_min,
        const double lambda_max,
        const double z0_min,
        const double z0_max);

      inline bool sameSign(double n1, double n2, double n3, double n4)
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

    private:
      QuadTreeLegendre m_cdcLegendreQuadTree; /**< Quad tree, which is used for finding stereohits with known polar angle */

    };
  }


}
