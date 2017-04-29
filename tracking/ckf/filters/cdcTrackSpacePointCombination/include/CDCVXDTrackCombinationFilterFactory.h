/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/BaseCDCTrackSpacePointCombinationFilter.h>

namespace Belle2 {
  /// Filter factory for detector track combination filters.
  class CDCVXDTrackCombinationFilterFactory : public TrackFindingCDC::FilterFactory<BaseCDCVXDTrackCombinationFilter > {

  private:
    /// Type of the base class
    typedef TrackFindingCDC::FilterFactory<BaseCDCVXDTrackCombinationFilter> Super;

  public:
    /// Constructor forwarding the default filter name
    CDCVXDTrackCombinationFilterFactory(const std::string& defaultFilterName = "all");

    /// Getter for a short identifier for the factory
    std::string getIdentifier() const override;

    /// Getter for a descriptive purpose of the constructed filters
    std::string getFilterPurpose() const override;

    /// Getter for valid filter names and a description for each
    std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

    /// Create a filter with the given name.
    std::unique_ptr<BaseCDCVXDTrackCombinationFilter> create(const std::string& filterName) const override;
  };
}
