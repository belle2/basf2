/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/ca/NeighborWeight.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <boost/range/iterator_range.hpp>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Helper class template to get a closest wire neighbor by its neighborhood relation type
    template<WireNeighborType NeighborType>
    struct NeighborWireGetter {
      /// Getter for the closest neighbor
      static inline const CDCWire* get(const CDCWire& /*wire*/)
      { return nullptr; }
    };

    /// Helper class to get the clockwise outwards wire neighbor
    template<>
    struct NeighborWireGetter<CW_OUT_NEIGHBOR> {
      /// Getter for the clockwise outwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCWOutwards(); }
    };

    /// Helper class to get the clockwise in wire neighbor
    template<>
    struct NeighborWireGetter<CW_NEIGHBOR> {
      /// Getter for the clockwise wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCW(); }
    };

    /// Helper class to get the clockwise inwards wire neighbor
    template<>
    struct NeighborWireGetter<CW_IN_NEIGHBOR> {
      /// Getter for the clockwise inwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCWInwards(); }
    };

    /// Helper class to get the counterclockwise inwards wire neighbor
    template<>
    struct NeighborWireGetter<CCW_IN_NEIGHBOR> {
      /// Getter for the the counterclockwise inwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCCWInwards(); }
    };


    /// Helper class to get the counterclockwise wire neighbor
    template<>
    struct NeighborWireGetter<CCW_NEIGHBOR> {
      /// Getter for the the counterclockwise wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCCW(); }
    };

    /// Helper class to get the counterclockwise outwards wire neighbor
    template<>
    struct NeighborWireGetter<CCW_OUT_NEIGHBOR> {
      /// Getter for the the counterclockwise outwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCCWOutwards(); }
    };


    /** Class mapping the neighborhood of wires to the neighborhood of wire hits
     *  Class providing the neighborhood filter interface to the NeighborhoodBuilder
     *  for the construction of wire neighborhoods
     */
    template<WireNeighborType ConcreteNeighborType>
    class WireHitRelationFilter : public Filter<Relation<CDCWireHit>> {

    public:
      /** Returns a two iterator range covering the range of possible neighboring wire hits of the
       *  given facet out of the sorted range given by the two other argumets.
       */
      template<class ACDCWireHitIterator>
      boost::iterator_range<ACDCWireHitIterator>
      getPossibleNeighbors(const CDCWireHit& wireHit,
                           const ACDCWireHitIterator& itBegin,
                           const ACDCWireHitIterator& itEnd) const
      {

        const CDCWire& wire = wireHit.getWire();
        const CDCWire* ptrNeighborWire = NeighborWireGetter<ConcreteNeighborType>::get(wire);

        if (ptrNeighborWire == nullptr) {
          return boost::iterator_range<ACDCWireHitIterator>(itEnd, itEnd);
        } else {
          const CDCWire& neighborWire = *ptrNeighborWire;
          std::pair<ACDCWireHitIterator, ACDCWireHitIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, neighborWire);
          return boost::iterator_range<ACDCWireHitIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);
        }
      }

      /** Legacy method */
      inline NeighborWeight isGoodNeighbor(const CDCWireHit& /*fromWireHit*/,
                                           const CDCWireHit& /*toWirehit*/)
      {
        return 0;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      inline CellWeight operator()(const Relation<CDCWireHit>& relation) override final
      {
        const CDCWireHit* ptrFrom = relation.first;
        const CDCWireHit* ptrTo = relation.second;
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return 0;
      }

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
