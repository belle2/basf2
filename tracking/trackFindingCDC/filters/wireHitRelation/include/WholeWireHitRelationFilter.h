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

        const int nWireNeighbors = 8 + 10 * (a_neighborhoodDegree - 1);
        std::vector<const CDCWire*> m_wireNeighbors;
        m_wireNeighbors.reserve(nWireNeighbors);

        std::vector<std::reference_wrapper<CDCWireHit> > m_wireHitNeighbors;
        m_wireHitNeighbors.reserve(2 * nWireNeighbors);

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
        const CDCWire& wire = wireHit.getWire();

        const CDCWire* ccwSixthSecondWireNeighbor = wireTopology.getSecondNeighborSixOClock(wire);
        const CDCWire* ccwInWireNeighbor = wireTopology.getNeighborCCWInwards(wire);
        const CDCWire* ccwWireNeighbor = wireTopology.getNeighborCCW(wire);
        const CDCWire* ccwOutWireNeighbor = wireTopology.getNeighborCCWOutwards(wire);
        const CDCWire* ccwTwelvethSecondWireNeighbor = wireTopology.getSecondNeighborTwelveOClock(wire);

        const CDCWire* cwSixthSecondWireNeighbor = ccwSixthSecondWireNeighbor;
        const CDCWire* cwInWireNeighbor = wireTopology.getNeighborCWInwards(wire);
        const CDCWire* cwWireNeighbor = wireTopology.getNeighborCW(wire);
        const CDCWire* cwOutWireNeighbor = wireTopology.getNeighborCWOutwards(wire);
        const CDCWire* cwTwelvethSecondWireNeighbor = ccwTwelvethSecondWireNeighbor;

        // Insert the neighbors such that they are most likely sorted.

        // Degree 1 neighnborhood - only add the six oclock and the twelve oclock neighbot once
        if (a_neighborhoodDegree > 1 and ccwSixthSecondWireNeighbor) m_wireNeighbors.push_back(ccwSixthSecondWireNeighbor);

        if (cwInWireNeighbor) m_wireNeighbors.push_back(cwInWireNeighbor);
        if (ccwInWireNeighbor) m_wireNeighbors.push_back(ccwInWireNeighbor);

        if (cwWireNeighbor) m_wireNeighbors.push_back(cwWireNeighbor);
        if (ccwWireNeighbor) m_wireNeighbors.push_back(ccwWireNeighbor);

        if (cwOutWireNeighbor) m_wireNeighbors.push_back(cwOutWireNeighbor);
        if (ccwOutWireNeighbor) m_wireNeighbors.push_back(ccwOutWireNeighbor);

        if (a_neighborhoodDegree > 1 and ccwTwelvethSecondWireNeighbor) m_wireNeighbors.push_back(ccwTwelvethSecondWireNeighbor);

        for (int degree = 1; degree < a_neighborhoodDegree; ++degree) {
          if (cwSixthSecondWireNeighbor) {
            cwSixthSecondWireNeighbor = cwSixthSecondWireNeighbor->getNeighborCW();
            m_wireNeighbors.push_back(cwSixthSecondWireNeighbor);
          }
          if (ccwSixthSecondWireNeighbor) {
            ccwSixthSecondWireNeighbor = ccwSixthSecondWireNeighbor->getNeighborCCW();
            m_wireNeighbors.push_back(ccwSixthSecondWireNeighbor);
          }

          if (cwInWireNeighbor) {
            cwInWireNeighbor = cwInWireNeighbor->getNeighborCW();
            m_wireNeighbors.push_back(cwInWireNeighbor);
          }
          if (ccwInWireNeighbor) {
            ccwInWireNeighbor = ccwInWireNeighbor->getNeighborCCW();
            m_wireNeighbors.push_back(ccwInWireNeighbor);
          }

          if (cwWireNeighbor) {
            cwWireNeighbor = cwWireNeighbor->getNeighborCW();
            m_wireNeighbors.push_back(cwWireNeighbor);
          }
          if (ccwWireNeighbor) {
            ccwWireNeighbor = ccwWireNeighbor->getNeighborCCW();
            m_wireNeighbors.push_back(ccwWireNeighbor);
          }

          if (cwOutWireNeighbor) {
            cwOutWireNeighbor = cwOutWireNeighbor->getNeighborCW();
            m_wireNeighbors.push_back(cwOutWireNeighbor);
          }
          if (ccwOutWireNeighbor) {
            ccwOutWireNeighbor = ccwOutWireNeighbor->getNeighborCCW();
            m_wireNeighbors.push_back(ccwOutWireNeighbor);
          }

          if (cwTwelvethSecondWireNeighbor) {
            cwTwelvethSecondWireNeighbor = cwTwelvethSecondWireNeighbor->getNeighborCW();
            m_wireNeighbors.push_back(cwTwelvethSecondWireNeighbor);
          }
          if (ccwTwelvethSecondWireNeighbor) {
            ccwTwelvethSecondWireNeighbor = ccwTwelvethSecondWireNeighbor->getNeighborCCW();
            m_wireNeighbors.push_back(ccwTwelvethSecondWireNeighbor);
          }
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
