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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    // Guard to prevent repeated instantiations
    extern template class Filter<Relation<const CDCSegmentPair> >;

    /// Base class for filtering the neighborhood of axial stereo segment pairs
    class BaseSegmentPairRelationFilter : public Filter<Relation<const CDCSegmentPair>> {

    public:
      /// Returns the segment pairs form the range that continue on the to site of the given segment pair.
      std::vector<const CDCSegmentPair*> getPossibleNeighbors(
        const CDCSegmentPair* segmentPair,
        const std::vector<const CDCSegmentPair*>::const_iterator& itBegin,
        const std::vector<const CDCSegmentPair*>::const_iterator& itEnd) const;

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all axial stereo segment pair neighbors.
       */
      virtual Weight operator()(const CDCSegmentPair& from, const CDCSegmentPair& to);

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      Weight operator()(const Relation<const CDCSegmentPair>& relation) override;
    };
  }
}
