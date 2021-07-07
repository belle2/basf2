/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/relations/SectorMapBasedVTXPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/dataobjects/FullSecID.h>

namespace Belle2 {
  void SectorMapBasedVTXPairFilter::beginRun()
  {
    Super::beginRun();

    m_vxdtfFilters = m_filtersContainer.getFilters(m_param_sectorMapName);
    if (not m_vxdtfFilters) {
      B2FATAL("Requested secMapName '" << m_param_sectorMapName << "' does not exist!");
    }
  }

  void SectorMapBasedVTXPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "sectorMapName"), m_param_sectorMapName,
                                  "Name of the sector map to use.", m_param_sectorMapName);
  }

  TrackFindingCDC::Weight SectorMapBasedVTXPairFilter::operator()(const std::pair<const CKFToVTXState*, const CKFToVTXState*>&
      relation)
  {
    const CKFToVTXState* fromState = relation.first;
    const CKFToVTXState* toState = relation.second;

    B2ASSERT("From and to state must be set", fromState and toState);

    const CKFToVTXState::stateCache& outerStateCache = fromState->getStateCache();
    const CKFToVTXState::stateCache& innerStateCache = toState->getStateCache();

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
