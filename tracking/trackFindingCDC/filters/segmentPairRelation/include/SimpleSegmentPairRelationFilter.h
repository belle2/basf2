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

    /// Class filtering the neighborhood of axial stereo segment pairs based on simple criteria.
    class SimpleSegmentPairRelationFilter :
      public BaseSegmentPairRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseSegmentPairRelationFilter;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:

      /// Main filter method weighting the relation between the segment pairs.
      virtual
      Weight operator()(const CDCSegmentPair& fromSegmentPair,
                        const CDCSegmentPair& toSegmentPair) override final;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
