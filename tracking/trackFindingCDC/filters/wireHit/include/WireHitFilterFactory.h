/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/wireHit/BaseWireHitFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseWireHitFilter>;

    /// Factory that can create appropriate wireHit filters from associated names.
    class WireHitFilterFactory : public FilterFactory<BaseWireHitFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseWireHitFilter>;

    public:
      /// Constructor forwarding the default filter name
      explicit WireHitFilterFactory(const std::string& defaultFilterName = "all");

      /// Default destructor
      ~WireHitFilterFactory();

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseWireHitFilter> create(const std::string& filterName) const override;
    };
  }
}
