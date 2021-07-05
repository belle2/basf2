/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    // Guard to prevent repeated instantiations
    extern template class RelationFilter<const CDCSegment2D>;

    /// Base class for filtering the neighborhood of segments
    class BaseSegmentRelationFilter : public RelationFilter<const CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = RelationFilter<const CDCSegment2D>;

    public:
      /// Default constructor
      BaseSegmentRelationFilter();

      /// Default destructor
      virtual ~BaseSegmentRelationFilter();

      /// Returns all equivalent segment in the range.
      std::vector<const CDCSegment2D*> getPossibleTos(
        const CDCSegment2D* from,
        const std::vector<const CDCSegment2D*>& segments) const final;

      using Super::operator();

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCSegment2D>& relation) override;
    };
  }
}
