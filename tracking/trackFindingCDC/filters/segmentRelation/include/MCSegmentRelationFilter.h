/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment relations based on MC information.
    class MCSegmentRelationFilter :
      public MCSymmetricFilterMixin<Filter<Relation<CDCRecoSegment2D> > > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<Relation<CDCRecoSegment2D> > > Super;

    public:
      /// Constructor
      MCSegmentRelationFilter(bool allowReverse = true) :
        Super(allowReverse)
      {}

      /// Checks if a segment relation is a good combination.
      virtual NeighborWeight operator()(const CDCRecoSegment2D& fromSegment,
                                        const CDCRecoSegment2D& toSegment) override final;

    }; // end class MCSegmentRelationFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
