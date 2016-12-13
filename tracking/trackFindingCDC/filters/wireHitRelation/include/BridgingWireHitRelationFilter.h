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

#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

#include <vector>
#include <map>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Wire hit relation filter that is compensating for hit inefficiencies.
     *
     *  The default setup counters some hits lost due to a drift time cut off
     *  if the track enters at the edge of a drift cell. The biggest effect observed in the CDC simulation
     *  was observed when the track is along the 2, 4, 8 and 10 o'clock direction since in this case
     *  *two* neighboring hits can be lost due to this time cut off.
     *  In an attempt to detect if this occured the primary wire hit neighborhood
     *  is slightly extended to bridge to the secondary neighborhood in the critical directions.
     *
     *  The criterium can be extended lower more for instance to include the
     *  secondary neighbors for each missing primary drift cell.
     */
    class BridgingWireHitRelationFilter : public Filter<Relation<const CDCWireHit> > {

    private:
      /// Type of the base class
      using Super = Filter<Relation<const CDCWireHit> >;

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        moduleParamList->addParameter(prefixed(prefix, "missingPrimaryNeighborThresholds"),
                                      m_param_missingPrimaryNeighborThresholdMap,
                                      "Map of o'clock directions to number of missing drift cells "
                                      "in the primary neighborhood to trigger the inclusion of secondary neighbors "
                                      "in that very o'clock direction",
                                      m_param_missingPrimaryNeighborThresholdMap
                                     );
      }

      void initialize() override
      {
        Super::initialize();
        for (short oClockDirection = 0; oClockDirection < 12; oClockDirection++) {
          m_missingPrimaryNeighborThresholds[oClockDirection] = 3;
          if (m_param_missingPrimaryNeighborThresholdMap.count(oClockDirection)) {
            m_missingPrimaryNeighborThresholds[oClockDirection] =
              m_param_missingPrimaryNeighborThresholdMap[oClockDirection];
          }
        }

        /// Prepare the lookup such that it turns out most likely sorted.
        for (short oClockDirection : {5, 6, 7, 4, 8, 3, 9, 2, 10, 1, 0, 11}) {
          if (m_missingPrimaryNeighborThresholds[oClockDirection] < 3) {
            m_consideredSecondaryNeighbors.push_back(oClockDirection);
          }
        }
      }

      /// Returns a vector containing the neighboring wire hits of the given wire hit out of the sorted range given by the two iterator other argumets.
      template<class ACDCWireHitIterator>
      std::vector<std::reference_wrapper<CDCWireHit> > getPossibleNeighbors(const CDCWireHit& wireHit,
          const ACDCWireHitIterator& itBegin,
          const ACDCWireHitIterator& itEnd)
      {
        std::vector<std::pair<const CDCWire*, int> > wireNeighbors;
        wireNeighbors.reserve(8);

        std::vector<std::reference_wrapper<CDCWireHit> > wireHitNeighbors;
        wireHitNeighbors.reserve(12);

        std::array<short, 12> missingPrimaryNeighbor = {0};

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

        const CDCWire& wire = wireHit.getWire();

        // Analyse primary neighborhood - sorted such that the wire hits relations are most likely sorted.
        for (short oClockDirection : {5, 7, 3, 9, 1, 11}) {
          MayBePtr<const CDCWire> neighborWire = wireTopology.getPrimaryNeighbor(oClockDirection, wire);
          if (neighborWire) wireNeighbors.emplace_back(neighborWire, oClockDirection);
        }

        std::sort(std::begin(wireNeighbors), std::end(wireNeighbors));

        for (std::pair<const CDCWire*, int> wireAndOClockDirection : wireNeighbors) {
          const CDCWire* neighborWire = wireAndOClockDirection.first;
          int oClockDirection = wireAndOClockDirection.second;

          Range<ACDCWireHitIterator> wireHitRange = std::equal_range(itBegin, itEnd, *neighborWire);
          if (wireHitRange.empty()) {
            int ccwOClockDirection = oClockDirection - 1;
            int cwOClockDirection = oClockDirection == 11 ? 0 : oClockDirection + 1;
            ++missingPrimaryNeighbor[ccwOClockDirection];
            ++missingPrimaryNeighbor[oClockDirection];
            ++missingPrimaryNeighbor[cwOClockDirection];
          }
          wireHitNeighbors.insert(wireHitNeighbors.end(),
                                  wireHitRange.begin(),
                                  wireHitRange.end());
        }

        size_t nPrimaryWireHitNeighbors = wireHitNeighbors.size();
        wireNeighbors.clear();

        // Analyse secondary neighborhood
        for (short oClockDirection : m_consideredSecondaryNeighbors) {
          MayBePtr<const CDCWire> neighborWire =
            wireTopology.getSecondaryNeighbor(oClockDirection, wire);
          if (not neighborWire) continue;
          if (missingPrimaryNeighbor[oClockDirection] < m_missingPrimaryNeighborThresholds[oClockDirection]) continue;
          wireNeighbors.emplace_back(neighborWire, oClockDirection);
        }

        std::sort(std::begin(wireNeighbors), std::end(wireNeighbors));

        for (std::pair<const CDCWire*, int> wireAndOClockDirection : wireNeighbors) {
          const CDCWire* neighborWire = wireAndOClockDirection.first;
          Range<ACDCWireHitIterator> wireHitRange = std::equal_range(itBegin, itEnd, *neighborWire);
          wireHitNeighbors.insert(wireHitNeighbors.end(),
                                  wireHitRange.begin(),
                                  wireHitRange.end());
        }

        /// Merge the sorted primary and secondary neighbors.
        std::inplace_merge(wireHitNeighbors.begin(),
                           wireHitNeighbors.begin() + nPrimaryWireHitNeighbors,
                           wireHitNeighbors.end(),
                           std::less<const CDCWireHit>());

        return wireHitNeighbors;
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

    private:
      /// Parameter: A map from o'clock direction to the number of missing primary drift cells
      std::map<int, int> m_param_missingPrimaryNeighborThresholdMap =
      {{0, 2}, {2, 2}, {4, 2}, {6, 2}, {8, 2}, {10, 2}};

      /// Array for the number of primary drift cells to be included for the o'clock position at each index.
      std::array<short, 12> m_missingPrimaryNeighborThresholds;

      /// Indices of the considered o'clock positions of the secondary neighborhood.
      std::vector<short> m_consideredSecondaryNeighbors;
    };
  }
}
