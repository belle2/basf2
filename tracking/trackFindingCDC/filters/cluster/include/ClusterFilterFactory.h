/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseClusterFilter>;

    /// Factory that can create appropriate cluster filters from associated names.
    class ClusterFilterFactory : public FilterFactory<BaseClusterFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseClusterFilter>;

    public:
      /// Constructor forwarding the default filter name
      ClusterFilterFactory(const std::string& defaultFilterName = "mva_bkg");

      /// Default destructor
      ~ClusterFilterFactory();

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseClusterFilter> create(const std::string& filterName) const override;
    };
  }
}
