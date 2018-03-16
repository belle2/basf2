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

#include <tracking/trackFindingCDC/filters/stereoHits/BaseStereoHitFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseStereoHitFilter>;

    /// Factory that can create appropriate stereo hit to track combination filters from associated names.
    class StereoHitFilterFactory : public FilterFactory<BaseStereoHitFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseStereoHitFilter>;

    public:
      /// Constructor forwarding the default filter name
      StereoHitFilterFactory(const std::string& defaultFilterName = "all");

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseStereoHitFilter> create(const std::string& filterName) const override;
    };
  }
}
