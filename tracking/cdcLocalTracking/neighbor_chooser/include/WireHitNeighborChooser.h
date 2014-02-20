/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef WIREHITNEIGHBORCHOOSER_H
#define WIREHITNEIGHBORCHOOSER_H

#include <boost/range/iterator_range.hpp>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/Compare.h>

#include <cmath>


namespace Belle2 {
  namespace CDCLocalTracking {

    ///Helper class template to get a closest wire neighbor by its neighborhood relation type
    template<WireNeighborType NeighborType>
    struct NeighborWireGetter {
      ///Getter for the closest neighbor
      static inline const CDCWire* get(const CDCWire& wire __attribute__((unused)))
      { return nullptr; }
    };

    ///Helper class to get the clockwise outwards wire neighbor
    template<>
    struct NeighborWireGetter<CW_OUT_NEIGHBOR> {
      ///Getter for the clockwise outwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCWOutwards(); }
    };

    ///Helper class to get the clockwise in wire neighbor
    template<>
    struct NeighborWireGetter<CW_NEIGHBOR> {
      ///Getter for the clockwise wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCW(); }
    };

    ///Helper class to get the clockwise inwards wire neighbor
    template<>
    struct NeighborWireGetter<CW_IN_NEIGHBOR> {
      ///Getter for the clockwise inwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCWInwards(); }
    };

    ///Helper class to get the counterclockwise inwards wire neighbor
    template<>
    struct NeighborWireGetter<CCW_IN_NEIGHBOR> {
      ///Getter for the the counterclockwise inwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCCWInwards(); }
    };


    ///Helper class to get the counterclockwise wire neighbor
    template<>
    struct NeighborWireGetter<CCW_NEIGHBOR> {
      ///Getter for the the counterclockwise wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCCW(); }
    };

    ///Helper class to get the counterclockwise outwards wire neighbor
    template<>
    struct NeighborWireGetter<CCW_OUT_NEIGHBOR> {
      ///Getter for the the counterclockwise outwards wire neighbor
      static inline const CDCWire* get(const CDCWire& wire)
      { return wire.getNeighborCCWOutwards(); }
    };


    ///Class mapping the neighborhood of wires to the neighborhood of wire hits
    /** Class providing the neighborhood filter interface to the NeighborhoodBuilder for the construction of wire neighborhoods*/
    template<WireNeighborType ConcreteNeighborType>
    class WireHitNeighborChooser {

    public:

      /// Empty constructor
      WireHitNeighborChooser() {;}

      /// Empty destructor
      ~WireHitNeighborChooser() {;}

      /// Clear remembered informations. Nothing to do in this case
      void clear() const {;}

      /// Forwards the initialize method from the module
      void initialize() {;}

      /// Forwards the terminate method from the module
      void terminate() {;}



      template<class CDCWireHitIterator>
      boost::iterator_range<CDCWireHitIterator> getPossibleNeighbors(const CDCWireHit& wireHit, const CDCWireHitIterator& itBegin, const CDCWireHitIterator& itEnd) const {

        const CDCWire& wire = wireHit.getWire();
        const CDCWire* ptrNeighborWire = NeighborWireGetter<ConcreteNeighborType>::get(wire);

        if (ptrNeighborWire == nullptr) {
          return boost::iterator_range<CDCWireHitIterator>(itEnd, itEnd);
        } else {
          const CDCWire& neighborWire = *ptrNeighborWire;
          std::pair<CDCWireHitIterator, CDCWireHitIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, neighborWire);
          return boost::iterator_range<CDCWireHitIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);
        }
      }

      /// Returns whether the wire hit is a good neighbor.
      /** Returns if the wire hit given in the range of possible neighbors is also a good neighbor.
       *  In the case of wire hits every neighbor is a good neighbor */
      inline Weight isGoodNeighbor(
        const CDCWireHit& wirehit __attribute__((unused)),
        const CDCWireHit& neighborWirehit __attribute__((unused))
      ) const {
        return 0; // All possible neighbors are good ones but the relation does not contribute any specific gain in points
      }

    }; // end class

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //WIREHITNEIGHBORCHOOSER_H
