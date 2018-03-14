/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>
#include <string>
#include <cmath>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCWireHit>;

WholeWireHitRelationFilter::WholeWireHitRelationFilter(int neighborhoodDegree)
  : m_param_degree(neighborhoodDegree)
{
}

WholeWireHitRelationFilter::~WholeWireHitRelationFilter() = default;

void WholeWireHitRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                  const std::string& prefix)
{
  moduleParamList
  ->addParameter(prefixed(prefix, "degree"),
                 m_param_degree,
                 "Neighbor degree which are included. 1 for primary, 2 for secondary, 3 ...",
                 m_param_degree);
}

std::vector<CDCWireHit*> WholeWireHitRelationFilter::getPossibleTos(
  CDCWireHit* from,
  const std::vector<CDCWireHit*>& wireHits) const
{
  assert(std::is_sorted(wireHits.begin(), wireHits.end(), LessOf<Deref>()) &&
         "Expected wire hits to be sorted");

  const int nWireNeighbors = 8 + 10 * (m_param_degree - 1);
  std::vector<const CDCWire*> m_wireNeighbors;
  m_wireNeighbors.reserve(nWireNeighbors);

  std::vector<CDCWireHit*> m_wireHitNeighbors;
  m_wireHitNeighbors.reserve(2 * nWireNeighbors);

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const CDCWire& wire = from->getWire();

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
  if (m_param_degree > 1 and ccwSixthSecondWireNeighbor)
    m_wireNeighbors.push_back(ccwSixthSecondWireNeighbor);

  if (cwInWireNeighbor) m_wireNeighbors.push_back(cwInWireNeighbor);
  if (ccwInWireNeighbor) m_wireNeighbors.push_back(ccwInWireNeighbor);

  if (cwWireNeighbor) m_wireNeighbors.push_back(cwWireNeighbor);
  if (ccwWireNeighbor) m_wireNeighbors.push_back(ccwWireNeighbor);

  if (cwOutWireNeighbor) m_wireNeighbors.push_back(cwOutWireNeighbor);
  if (ccwOutWireNeighbor) m_wireNeighbors.push_back(ccwOutWireNeighbor);

  if (m_param_degree > 1 and ccwTwelvethSecondWireNeighbor)
    m_wireNeighbors.push_back(ccwTwelvethSecondWireNeighbor);

  for (int degree = 1; degree < m_param_degree; ++degree) {
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

  for (const CDCWire* neighborWire : m_wireNeighbors) {
    ConstVectorRange<CDCWireHit*> neighborWireHits{
      std::equal_range(wireHits.begin(), wireHits.end(), neighborWire, LessOf<Deref>())};

    m_wireHitNeighbors.insert(m_wireHitNeighbors.end(),
                              neighborWireHits.begin(),
                              neighborWireHits.end());
  }

  return m_wireHitNeighbors;
}
