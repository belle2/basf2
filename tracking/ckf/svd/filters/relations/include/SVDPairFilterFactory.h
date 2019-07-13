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

#include <tracking/ckf/svd/filters/relations/BaseSVDPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  /// Factory that can create appropriate cluster filters from associated names.
  class SVDPairFilterFactory : public TrackFindingCDC::FilterFactory<BaseSVDPairFilter> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::FilterFactory<BaseSVDPairFilter>;

  public:
    /// Constructor forwarding the default filter name
    explicit SVDPairFilterFactory(const std::string& defaultFilterName = "sensor");

    /// Default destructor
    ~SVDPairFilterFactory();

    /// Getter for a short identifier for the factory
    std::string getIdentifier() const override;

    /// Getter for a descriptive purpose of the constructed filters
    std::string getFilterPurpose() const override;

    /// Getter for valid filter names and a description for each
    std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

    /// Create a filter with the given name.
    std::unique_ptr<BaseSVDPairFilter> create(const std::string& filterName) const override;
  };
}
