/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef WHOLEWIREHITNEIGHBORCHOOSER_H
#define WHOLEWIREHITNEIGHBORCHOOSER_H

#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>
#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>
#include <boost/range/iterator_range.hpp>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class mapping the neighborhood of wires to the neighborhood of wire hits.
    /** Class providing the neighborhood filter interface to the NeighborhoodBuilder for the construction of wire neighborhoods.*/
    template<bool withSecondaryNeighborhood = false>
    class WholeWireHitNeighborChooser {

    public:
      /// Empty constructor
      WholeWireHitNeighborChooser()
      {
        m_wireNeighbors.reserve(withSecondaryNeighborhood ? 18 : 6);
        m_wireHitNeighbors.reserve(withSecondaryNeighborhood ? 32 : 12);
      }

      /// Clear for interface compliance. Does nothing here.
      void clear() {}

      /// Returns a vector containing the neighboring wire hits of the given wire hit out of the sorted range given by the two iterator other argumets.
      template<class CDCWireHitIterator>
      const std::vector<const CDCWireHit*>& getPossibleNeighbors(const CDCWireHit* ptrWireHit,
                                                                 const CDCWireHitIterator& itBegin,
                                                                 const CDCWireHitIterator& itEnd)
      {
        m_wireNeighbors.clear();
        m_wireHitNeighbors.clear();
        if (not ptrWireHit) return m_wireHitNeighbors;
        const CDCWireHit& wireHit = *ptrWireHit;

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

        const CDCWire& wire = wireHit.getWire();
        m_wireNeighbors.reserve(withSecondaryNeighborhood ? 18 : 6);

        const CDCWire* ccwInWireNeighborPtr = wireTopology.getNeighborCCWInwards(wire);
        const CDCWire* cwInWireNeighborPtr = wireTopology.getNeighborCWInwards(wire);

        const CDCWire* ccwWireNeighborPtr = wireTopology.getNeighborCCW(wire);
        const CDCWire* cwWireNeighborPtr = wireTopology.getNeighborCW(wire);

        const CDCWire* ccwOutWireNeighborPtr = wireTopology.getNeighborCCWOutwards(wire);
        const CDCWire* cwOutWireNeighborPtr = wireTopology.getNeighborCWOutwards(wire);

        if (withSecondaryNeighborhood) {

          const CDCWire* oneSecondWireNeighborPtr = wireTopology.getSecondNeighborOneOClock(wire);
          const CDCWire* twoSecondWireNeighborPtr = wireTopology.getSecondNeighborTwoOClock(wire);
          const CDCWire* threeSecondWireNeighborPtr = wireTopology.getSecondNeighborThreeOClock(wire);
          const CDCWire* fourSecondWireNeighborPtr = wireTopology.getSecondNeighborFourOClock(wire);
          const CDCWire* fiveSecondWireNeighborPtr = wireTopology.getSecondNeighborFiveOClock(wire);
          const CDCWire* sixSecondWireNeighborPtr = wireTopology.getSecondNeighborSixOClock(wire);
          const CDCWire* sevenSecondWireNeighborPtr = wireTopology.getSecondNeighborSevenOClock(wire);
          const CDCWire* eightSecondWireNeighborPtr = wireTopology.getSecondNeighborEightOClock(wire);
          const CDCWire* nineSecondWireNeighborPtr = wireTopology.getSecondNeighborNineOClock(wire);
          const CDCWire* tenSecondWireNeighborPtr = wireTopology.getSecondNeighborTenOClock(wire);
          const CDCWire* elevenSecondWireNeighborPtr = wireTopology.getSecondNeighborElevenOClock(wire);
          const CDCWire* twelveSecondWireNeighborPtr = wireTopology.getSecondNeighborTwelveOClock(wire);
          // Insert the neighbors such that they are most likely sorted.
          if (fiveSecondWireNeighborPtr) m_wireNeighbors.push_back(fiveSecondWireNeighborPtr);
          if (sixSecondWireNeighborPtr) m_wireNeighbors.push_back(sixSecondWireNeighborPtr);
          if (sevenSecondWireNeighborPtr) m_wireNeighbors.push_back(sevenSecondWireNeighborPtr);

          if (fourSecondWireNeighborPtr) m_wireNeighbors.push_back(fourSecondWireNeighborPtr);
          if (cwInWireNeighborPtr) m_wireNeighbors.push_back(cwInWireNeighborPtr);
          if (ccwInWireNeighborPtr) m_wireNeighbors.push_back(ccwInWireNeighborPtr);
          if (eightSecondWireNeighborPtr) m_wireNeighbors.push_back(eightSecondWireNeighborPtr);

          if (threeSecondWireNeighborPtr) m_wireNeighbors.push_back(threeSecondWireNeighborPtr);
          if (cwWireNeighborPtr) m_wireNeighbors.push_back(cwWireNeighborPtr);
          if (ccwWireNeighborPtr) m_wireNeighbors.push_back(ccwWireNeighborPtr);
          if (nineSecondWireNeighborPtr) m_wireNeighbors.push_back(nineSecondWireNeighborPtr);

          if (twoSecondWireNeighborPtr) m_wireNeighbors.push_back(twoSecondWireNeighborPtr);
          if (cwOutWireNeighborPtr) m_wireNeighbors.push_back(cwOutWireNeighborPtr);
          if (ccwOutWireNeighborPtr) m_wireNeighbors.push_back(ccwOutWireNeighborPtr);
          if (tenSecondWireNeighborPtr) m_wireNeighbors.push_back(tenSecondWireNeighborPtr);

          if (oneSecondWireNeighborPtr) m_wireNeighbors.push_back(oneSecondWireNeighborPtr);
          if (twelveSecondWireNeighborPtr) m_wireNeighbors.push_back(twelveSecondWireNeighborPtr);
          if (elevenSecondWireNeighborPtr) m_wireNeighbors.push_back(elevenSecondWireNeighborPtr);

        } else {

          // Insert the neighbors such that they are most likely sorted.
          if (cwInWireNeighborPtr) m_wireNeighbors.push_back(cwInWireNeighborPtr);
          if (ccwInWireNeighborPtr) m_wireNeighbors.push_back(ccwInWireNeighborPtr);

          if (cwWireNeighborPtr) m_wireNeighbors.push_back(cwWireNeighborPtr);
          if (ccwWireNeighborPtr) m_wireNeighbors.push_back(ccwWireNeighborPtr);

          if (cwOutWireNeighborPtr) m_wireNeighbors.push_back(cwOutWireNeighborPtr);
          if (ccwOutWireNeighborPtr) m_wireNeighbors.push_back(ccwOutWireNeighborPtr);

        }


        std::sort(std::begin(m_wireNeighbors), std::end(m_wireNeighbors));

        for (const CDCWire* ptrNeighborWire : m_wireNeighbors) {
          const CDCWire& neighborWire = *ptrNeighborWire;
          std::pair<CDCWireHitIterator, CDCWireHitIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, neighborWire);

          m_wireHitNeighbors.insert(m_wireHitNeighbors.end(),
                                    itPairPossibleNeighbors.first,
                                    itPairPossibleNeighbors.second);
        }

        return m_wireHitNeighbors;

      }

      /// Returns whether the wire hit is a good neighbor.
      /** Returns if the wire hit given in the range of possible neighbors is also a good neighbor.
       *  In the case of wire hits every neighbor is a good neighbor */
      inline NeighborWeight isGoodNeighbor(
        const CDCWireHit* wirehit __attribute__((unused)),
        const CDCWireHit* neighborWirehit __attribute__((unused))
      ) const
      {
        return 0; // All possible neighbors are good ones but the relation does not contribute any specific gain in points
      }

    private:
      // Storage for the current neighboring wires.
      std::vector<const CDCWire* > m_wireNeighbors;

      /// Storage for the current neighbors wire hits.
      std::vector<const CDCWireHit*> m_wireHitNeighbors;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //WHOLEWIREHITNEIGHBORCHOOSER_H
