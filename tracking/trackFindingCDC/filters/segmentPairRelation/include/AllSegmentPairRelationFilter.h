/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Relation filter that lets all possible combinations pass.
    class AllSegmentPairRelationFilter :
      public Filter<Relation<const CDCSegmentPair>>  {

    private:
      /// Type of the super class
      typedef Filter<Relation<const CDCSegmentPair>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// Implementation accepting all possible neighbors. Weight subtracts the overlap penalty.
      virtual
      Weight operator()(const CDCSegmentPair& fromPair,
                        const CDCSegmentPair& toPair) override final;

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
