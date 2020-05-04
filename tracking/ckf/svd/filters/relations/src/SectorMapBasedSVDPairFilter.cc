/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SectorMapBasedSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/dataobjects/FullSecID.h>

namespace Belle2 {
  void SectorMapBasedSVDPairFilter::beginRun()
  {
    Super::beginRun();

    m_vxdtfFilters = m_filtersContainer.getFilters(m_param_sectorMapName);
    if (not m_vxdtfFilters) {
      B2FATAL("Requested secMapName '" << m_param_sectorMapName << "' does not exist!");
    }
  }

  void SectorMapBasedSVDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "sectorMapName"), m_param_sectorMapName,
                                  "Name of the sector map to use.", m_param_sectorMapName);
  }

  TrackFindingCDC::Weight SectorMapBasedSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>&
      relation)
  {
    const CKFToSVDState* fromState = relation.first;
    const CKFToSVDState* toState = relation.second;

    B2ASSERT("From and to state must be set", fromState and toState);

    const CKFToSVDState::stateCache& outerStateCache = fromState->getStateCache();
    const CKFToSVDState::stateCache& innerStateCache = toState->getStateCache();

    B2ASSERT("Both hits must be present!", outerStateCache.isHitState and innerStateCache.isHitState);

    // TODO maybe it would be better to look for the full IDs first; maybe not
    B2ASSERT("Outer hit is invalid",
             m_vxdtfFilters->areCoordinatesValid(outerStateCache.sensorID, outerStateCache.localNormalizedu, outerStateCache.localNormalizedv));
    B2ASSERT("Inner hit is invalid",
             m_vxdtfFilters->areCoordinatesValid(innerStateCache.sensorID, innerStateCache.localNormalizedu, innerStateCache.localNormalizedv));

    FullSecID outerSecID = m_vxdtfFilters->getFullID(outerStateCache.sensorID, outerStateCache.localNormalizedu,
                                                     outerStateCache.localNormalizedv);
    FullSecID innerSecID = m_vxdtfFilters->getFullID(innerStateCache.sensorID, innerStateCache.localNormalizedu,
                                                     innerStateCache.localNormalizedv);

    const auto* outerStaticSector = m_vxdtfFilters->getStaticSector(outerSecID);

    const auto* filter = outerStaticSector->getFilter2sp(innerSecID);
    if (not filter) {
      return NAN;
    }

    const SpacePoint* outerHit = fromState->getHit();
    const SpacePoint* innerHit = toState->getHit();

    if (not filter->accept(*outerHit, *innerHit)) {
      return NAN;
    }

    B2INFO(outerSecID.getVxdID() << " -> " << innerSecID.getVxdID());
    return 1;
  }
}
