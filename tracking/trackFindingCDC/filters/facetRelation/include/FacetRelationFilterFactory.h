/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseFacetRelationFilter>;

    /// Factory that can create appropriate facet relation filters from associated names.
    class FacetRelationFilterFactory : public FilterFactory<BaseFacetRelationFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseFacetRelationFilter>;

    public:
      /// Constructor forwarding the default filter name
      explicit FacetRelationFilterFactory(const std::string& defaultFilterName = "chi2");

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseFacetRelationFilter> create(const std::string& filterName) const override;
    };
  }
}
