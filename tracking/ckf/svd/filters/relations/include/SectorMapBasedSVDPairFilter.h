/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/relations/BaseSVDPairFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /// Filter for relations between CKF SVD states based on SectorMaps
  class SectorMapBasedSVDPairFilter : public BaseSVDPairFilter {
    /// The parent class
    using Super = BaseSVDPairFilter;

    /// The VXDTF filter
    using SectorMapFilter = VXDTFFilters<SpacePoint>;
    /// The filter container
    using FilterContainer = FiltersContainer<SpacePoint>;

  public:
    /// default constructor to silence cppcheck warnings
    SectorMapBasedSVDPairFilter() = default;

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Give a final weight to the possibilities by asking the filter.
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation) override;

    /// Initialize the sector map
    void beginRun() final;

  private:
    /// The filter container to use
    FilterContainer& m_filtersContainer = FiltersContainer<SpacePoint>::getInstance();
    /// The sector map filter, will be set in begin run.
    SectorMapFilter* m_vxdtfFilters = nullptr;
    /// Name of the sector map to use
    std::string m_param_sectorMapName = "SVDOnlyDefault";
  };
}
