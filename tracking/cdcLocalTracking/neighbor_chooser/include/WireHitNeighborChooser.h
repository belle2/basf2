/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef WIREHITNEIGHBORCHOOSER_H_
#define WIREHITNEIGHBORCHOOSER_H_

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
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
      WireHitNeighborChooser()
      {;}

      /// Empty destructor
      ~WireHitNeighborChooser() {;}

      /// Clear remembered informations. Nothing to do in this case
      void clear() const {
        m_lastLowestPossibleNeighborValid = false;
      }

      /// Getter for the lowest possible neighbor of the wirehit.
      /** Returns the wire hit of the based on the neighbor wire */
      inline const CDCWireHit getLowestPossibleNeighbor(const CDCWireHit& wirehit) const {
        const CDCWire& wire = wirehit.getWire();
        const CDCWire* neighborWire = NeighborWireGetter<ConcreteNeighborType>::get(wire);
        if (neighborWire == nullptr) {

          m_lastLowestPossibleNeighborValid = false;
          neighborWire = &(CDCWire::getLowest());

        } else {

          m_lastLowestPossibleNeighborValid = true;

        }
        return CDCWireHit::getLowerBound(neighborWire);

      }
      /// Returns if the wire hit given is still in the possible neighbor range or not */
      inline bool
      isStillPossibleNeighbor(
        const CDCWireHit& wirehit __attribute__((unused)),
        const CDCWireHit& neighborWirehit,
        const CDCWireHit& lowestPossibleNeighbor
      ) const {

        return  m_lastLowestPossibleNeighborValid and
                neighborWirehit.getWire() == lowestPossibleNeighbor.getWire();

      }


      /// Returns whether the wire hit is a good neighbor.
      /** Returns if the wire hit given in the range of possible neighbors is also a good neighbor.
       *  In the case of wire hits every neighbor is a good neighbor */
      inline Weight isGoodNeighbor(
        const CDCWireHit& wirehit __attribute__((unused)),
        const CDCWireHit& neighborWirehit __attribute__((unused)),
        const CDCWireHit& lowestPossibleNeighbor __attribute__((unused))
      ) const {
        return 0; // All possible neighbors are good ones but the relation does not contribute any specific gain in points
      }

    private:
      mutable bool m_lastLowestPossibleNeighborValid;

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //WIREHITNEIGHBORCHOOSER_H_
