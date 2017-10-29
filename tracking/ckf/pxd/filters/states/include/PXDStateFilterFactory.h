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

#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

#include <tracking/ckf/pxd/filters/states/BasePXDStateFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  /// Factory that can create appropriate cluster filters from associated names.
  class PXDStateFilterFactory : public TrackFindingCDC::FilterFactory<BasePXDStateFilter> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::FilterFactory<BasePXDStateFilter>;

  public:
    /// Constructor forwarding the default filter name
    PXDStateFilterFactory(const std::string& defaultFilterName = "all");

    /// Default destructor
    ~PXDStateFilterFactory();

    /// Getter for a short identifier for the factory
    std::string getIdentifier() const override;

    /// Getter for a descriptive purpose of the constructed filters
    std::string getFilterPurpose() const override;

    /// Getter for valid filter names and a description for each
    std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

    /// Create a filter with the given name.
    std::unique_ptr<BasePXDStateFilter> create(const std::string& filterName) const override;
  };
}