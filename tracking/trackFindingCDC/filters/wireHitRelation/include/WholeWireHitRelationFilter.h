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

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <boost/range/iterator_range.hpp>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class mapping the neighborhood of wires to the neighborhood of wire hits.
    template <int a_neighborhoodDegree>
    class WholeWireHitRelationFilter : public Filter<Relation<const CDCWireHit>> {

    public:
      /// Returns a vector containing the neighboring wire hits of the given wire hit out of the sorted range given by the two iterator other argumets.
      template<class ACDCWireHitIterator>
      std::vector<std::reference_wrapper<CDCWireHit> > getPossibleNeighbors(const CDCWireHit& wireHit,
          const ACDCWireHitIterator& itBegin,
          const ACDCWireHitIterator& itEnd)
      {
        std::vector<const CDCWire* > m_wireNeighbors;
        m_wireNeighbors.reserve((a_neighborhoodDegree > 1) ? 18 : 6);

        std::vector<std::reference_wrapper<CDCWireHit> > m_wireHitNeighbors;
        m_wireHitNeighbors.reserve((a_neighborhoodDegree > 1) ? 32 : 12);

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

        const CDCWire& wire = wireHit.getWire();
        const CDCWire* ccwInWireNeighborPtr = wireTopology.getNeighborCCWInwards(wire);
        const CDCWire* cwInWireNeighborPtr = wireTopology.getNeighborCWInwards(wire);

        const CDCWire* ccwWireNeighborPtr = wireTopology.getNeighborCCW(wire);
        const CDCWire* cwWireNeighborPtr = wireTopology.getNeighborCW(wire);

        const CDCWire* ccwOutWireNeighborPtr = wireTopology.getNeighborCCWOutwards(wire);
        const CDCWire* cwOutWireNeighborPtr = wireTopology.getNeighborCWOutwards(wire);

        if (a_neighborhoodDegree > 1) {

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
          std::pair<ACDCWireHitIterator, ACDCWireHitIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, neighborWire);

          m_wireHitNeighbors.insert(m_wireHitNeighbors.end(),
                                    itPairPossibleNeighbors.first,
                                    itPairPossibleNeighbors.second);
        }

        return m_wireHitNeighbors;

      }

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCWireHit>& relation) final {
        const CDCWireHit * ptrFrom(relation.first);
        const CDCWireHit * ptrTo(relation.second);
        if (not ptrFrom or not ptrTo) return NAN;
        return 0;
      }
    };
  }
}
