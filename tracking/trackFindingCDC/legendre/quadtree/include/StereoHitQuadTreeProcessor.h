/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#pragma once
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for StereoTrackHits
     * The two axis are the inverse slope in z direction and the z0.
     * We loop over the z0 and calculate the z-slope with the x-y-z-position the recoHit has.
     *  */
    class StereoHitQuadTreeProcessor final : public QuadTreeProcessorTemplate<float, float, const CDCRecoHit3D, 2, 2> {

    public:
      StereoHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges,
                                 bool debugOutput = false) : QuadTreeProcessorTemplate(lastLevel, ranges, debugOutput, false)
      {
      }

      /**
       * Do only insert the hit into a node if the slope and z information calculated from this hit belongs into this node
       */
      bool insertItemInNode(QuadTree* node, const CDCRecoHit3D* hit, unsigned int slope_index,
                            unsigned int z0_index) const override final;
    };
  }
}
