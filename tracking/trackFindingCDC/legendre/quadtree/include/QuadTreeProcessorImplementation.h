/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#pragma once
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>

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
      bool insertItemInNode(QuadTree* node, const CDCRecoHit3D* hit, unsigned int /*slope_index*/,
                            unsigned int /*z0_index*/) const override final;
    };


    /** A QuadTreeProcessor for RecoSegments */
    class QuadTreeProcessorSegments final : public QuadTreeProcessorTemplate<int, float, CDCRecoSegment2D, 2, 2> {

    public:
      /**
       * The conformalTransformationPostion is the point which we use as an origin for the conformal transformation.
       */
      QuadTreeProcessorSegments(unsigned char lastLevel, const ChildRanges& ranges, const Vector2D& conformalTransformationPosition,
                                bool debugOutput = false) :
        QuadTreeProcessorTemplate(lastLevel, ranges, debugOutput), m_origin(conformalTransformationPosition) {}

      /**
       * Insert only if the sinogram of the front or the back part of the segment goes through the node.
       * Also we can intersect the sinograms of the front and the back and get an intersection point, which should lay in the node also.
       */
      bool insertItemInNode(QuadTree* node, CDCRecoSegment2D* recoItem, unsigned int /*t_index*/,
                            unsigned int /*r_index*/) const override final;
    private:
      Vector2D m_origin; /**< The origin of the conformal transformation */
    };

  }
}
