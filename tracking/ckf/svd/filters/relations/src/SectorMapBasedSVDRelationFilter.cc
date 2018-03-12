/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SectorMapBasedSVDRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/dataobjects/FullSecID.h>

namespace Belle2 {
  void SectorMapBasedSVDRelationFilter::beginRun()
  {
    Super::beginRun();

    m_vxdtfFilters = m_filtersContainer.getFilters(m_param_sectorMapName);
    if (not m_vxdtfFilters) {
      B2FATAL("Requested secMapName '" << m_param_sectorMapName << "' does not exist!");
    }
  }

  std::vector<CKFToSVDState*>
  SectorMapBasedSVDRelationFilter::getPossibleTos(CKFToSVDState* currentState,
                                                  const std::vector<CKFToSVDState*>& states) const
  {
    return states;
  }

  void SectorMapBasedSVDRelationFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "sectorMapName"), m_param_sectorMapName,
                                  "Name of the sector map to use.", m_param_sectorMapName);
  }

  TrackFindingCDC::Weight SectorMapBasedSVDRelationFilter::operator()(const CKFToSVDState& from, const CKFToSVDState& to)
  {
    const SpacePoint* outerHit = from.getHit();
    const SpacePoint* innerHit = to.getHit();

    B2ASSERT("Both hits must be present!", outerHit and innerHit);

    // TODO maybe it would be better to look for the full IDs first; maybe not
    B2ASSERT("Outer hit is invalid",
             m_vxdtfFilters->areCoordinatesValid(outerHit->getVxdID(), outerHit->getNormalizedLocalU(), outerHit->getNormalizedLocalV()));
    B2ASSERT("Inner hit is invalid",
             m_vxdtfFilters->areCoordinatesValid(innerHit->getVxdID(), innerHit->getNormalizedLocalU(), innerHit->getNormalizedLocalV()));

    FullSecID outerSecID = m_vxdtfFilters->getFullID(outerHit->getVxdID(), outerHit->getNormalizedLocalU(),
                                                     outerHit->getNormalizedLocalV());
    FullSecID innerSecID = m_vxdtfFilters->getFullID(innerHit->getVxdID(), innerHit->getNormalizedLocalU(),
                                                     innerHit->getNormalizedLocalV());

    const auto* outerStaticSector = m_vxdtfFilters->getStaticSector(outerSecID);

    const auto* filter = outerStaticSector->getFilter2sp(innerSecID);
    return filter->accept(*outerHit, *innerHit);
  }
}