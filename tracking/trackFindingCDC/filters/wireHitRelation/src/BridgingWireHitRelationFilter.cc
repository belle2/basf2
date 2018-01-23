/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>
#include <string>
#include <memory>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCWireHit>;

BridgingWireHitRelationFilter::BridgingWireHitRelationFilter() = default;

BridgingWireHitRelationFilter::~BridgingWireHitRelationFilter() = default;

void BridgingWireHitRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                     const std::string& prefix)
{
  moduleParamList
  ->addParameter(prefixed(prefix, "missingPrimaryNeighborThresholds"),
                 m_param_missingPrimaryNeighborThresholdMap,
                 "Map of o'clock directions to number of missing drift cells "
                 "in the primary neighborhood to trigger the inclusion of secondary neighbors "
                 "in that very o'clock direction",
                 m_param_missingPrimaryNeighborThresholdMap);
}

void BridgingWireHitRelationFilter::initialize()
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

std::vector<CDCWireHit*> BridgingWireHitRelationFilter::getPossibleTos(
  CDCWireHit* from,
  const std::vector<CDCWireHit*>& wireHits) const
{
  assert(std::is_sorted(wireHits.begin(), wireHits.end(), LessOf<Deref>()) &&
         "Expected wire hits to be sorted");

  std::vector<std::pair<const CDCWire*, int>> wireNeighbors;
  wireNeighbors.reserve(8);

  std::vector<CDCWireHit*> wireHitNeighbors;
  wireHitNeighbors.reserve(12);

  std::array<short, 12> missingPrimaryNeighbor = {0};

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const CDCWire& wire = from->getWire();

  // Analyse primary neighborhood - sorted such that the wire hits relations are most likely sorted.
  for (short oClockDirection : {5, 7, 3, 9, 1, 11}) {
    MayBePtr<const CDCWire> neighborWire = wireTopology.getPrimaryNeighbor(oClockDirection, wire);
    if (neighborWire) wireNeighbors.emplace_back(neighborWire, oClockDirection);
  }

  std::sort(std::begin(wireNeighbors), std::end(wireNeighbors));

  for (std::pair<const CDCWire*, int> wireAndOClockDirection : wireNeighbors) {
    const CDCWire* neighborWire = wireAndOClockDirection.first;
    int oClockDirection = wireAndOClockDirection.second;

    ConstVectorRange<CDCWireHit*> wireHitRange{
      std::equal_range(wireHits.begin(), wireHits.end(), neighborWire, LessOf<Deref>())};
    if (wireHitRange.empty()) {
      int ccwOClockDirection = oClockDirection - 1;
      int cwOClockDirection = oClockDirection == 11 ? 0 : oClockDirection + 1;
      ++missingPrimaryNeighbor[ccwOClockDirection];
      ++missingPrimaryNeighbor[oClockDirection];
      ++missingPrimaryNeighbor[cwOClockDirection];
    }
    wireHitNeighbors.insert(wireHitNeighbors.end(), wireHitRange.begin(), wireHitRange.end());
  }

  size_t nPrimaryWireHitNeighbors = wireHitNeighbors.size();
  wireNeighbors.clear();

  // Analyse secondary neighborhood
  for (short oClockDirection : m_consideredSecondaryNeighbors) {
    MayBePtr<const CDCWire> neighborWire = wireTopology.getSecondaryNeighbor(oClockDirection, wire);
    if (not neighborWire) continue;
    if (missingPrimaryNeighbor[oClockDirection] <
        m_missingPrimaryNeighborThresholds[oClockDirection])
      continue;
    wireNeighbors.emplace_back(neighborWire, oClockDirection);
  }

  std::sort(std::begin(wireNeighbors), std::end(wireNeighbors));

  for (std::pair<const CDCWire*, int> wireAndOClockDirection : wireNeighbors) {
    const CDCWire* neighborWire = wireAndOClockDirection.first;
    ConstVectorRange<CDCWireHit*> wireHitRange{
      std::equal_range(wireHits.begin(), wireHits.end(), neighborWire, LessOf<Deref>())};
    wireHitNeighbors.insert(wireHitNeighbors.end(), wireHitRange.begin(), wireHitRange.end());
  }

  /// Merge the sorted primary and secondary neighbors.
  std::inplace_merge(wireHitNeighbors.begin(),
                     wireHitNeighbors.begin() + nPrimaryWireHitNeighbors,
                     wireHitNeighbors.end(),
                     std::less<CDCWireHit*>());

  return wireHitNeighbors;
}
