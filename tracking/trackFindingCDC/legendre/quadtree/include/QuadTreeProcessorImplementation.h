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

namespace Belle2 {
  namespace TrackFindingCDC {
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

      /// Function to check whether sinogram is crossing the node (see AxialHitQuadTreeProcessor::insertItemInNode())
      bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

    };

  }
}
