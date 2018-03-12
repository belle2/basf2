/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /// Filter for relations between CKF SVD states based on SectorMaps
  class SectorMapBasedSVDRelationFilter : public TrackFindingCDC::RelationFilter<CKFToSVDState> {
    /// The parent class
    using Super = TrackFindingCDC::RelationFilter<CKFToSVDState>;

    /// The VXDTF filter
    using SectorMapFilter = VXDTFFilters<SpacePoint>;
    /// The filter container
    using FilterContainer = FiltersContainer<SpacePoint>;

  public:
    using Super::operator();

    /// Return all states.
    std::vector<CKFToSVDState*> getPossibleTos(CKFToSVDState* from,
                                               const std::vector<CKFToSVDState*>& states) const final;

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Give a final weight to the possibilities by asking the filter.
    TrackFindingCDC::Weight operator()(const CKFToSVDState& from, const CKFToSVDState& to) final;

    /// Initialize the sector map
    void beginRun() final;

  private:
    /// The filter container to use
    FilterContainer& m_filtersContainer = FiltersContainer<SpacePoint>::getInstance();
    /// The sector map filter, will be set in begin run.
    SectorMapFilter* m_vxdtfFilters = nullptr;

    /// Name of the sector map to use
    std::string m_param_sectorMapName;
  };
}
