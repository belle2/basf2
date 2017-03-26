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

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base class for filtering the neighborhood of segments
    class BaseSegmentRelationFilter : public Filter<Relation<const CDCSegment2D>> {

    public:
      /// Returns the full range of segments.
      template<class ASegmentIterator>
      boost::iterator_range<ASegmentIterator>
      getPossibleNeighbors(const CDCSegment2D& segment,
                           const ASegmentIterator& itBegin,
                           const ASegmentIterator& itEnd) const
      {
        auto compareISuperCluster = [](const CDCSegment2D & lhs, const CDCSegment2D & rhs) {
          return lhs.getISuperCluster() < rhs.getISuperCluster();
        };
        std::pair<ASegmentIterator, ASegmentIterator> sameSuperClusterItPair =
          std::equal_range(itBegin, itEnd, segment, compareISuperCluster);
        return boost::iterator_range<ASegmentIterator>(sameSuperClusterItPair.first,
                                                       sameSuperClusterItPair.second);
      }

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all segment neighbors.
       */
      virtual Weight operator()(const CDCSegment2D& from  __attribute__((unused)),
                                const CDCSegment2D& to  __attribute__((unused)))
      {
        return 1;
      }

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCSegment2D>& relation) override
      {
        const CDCSegment2D* ptrFrom(relation.first);
        const CDCSegment2D* ptrTo(relation.second);
        if (ptrFrom == ptrTo) return NAN; // Prevent relation to same.
        if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;

        // Make an overlap check to prevent aliases and reverse segments to be linked
        std::vector<const CDCWireHit*> fromWireHits;
        fromWireHits.reserve(ptrFrom->size());
        for (const CDCRecoHit2D& recoHit2D : *ptrFrom) {
          fromWireHits.push_back(&recoHit2D.getWireHit());
        }
        std::sort(fromWireHits.begin(), fromWireHits.end());
        int nOverlap = 0;
        for (const CDCRecoHit2D& recoHit2D : *ptrTo) {
          if (std::binary_search(fromWireHits.begin(), fromWireHits.end(), &recoHit2D.getWireHit())) {
            ++nOverlap;
          }
        }

        if (1.0 * nOverlap / ptrFrom->size() > 0.8 or 1.0 * nOverlap / ptrFrom->size() > 0.8) {
          return NAN;
        }

        return operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
