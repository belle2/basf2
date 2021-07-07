/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseAxialSegmentPairFilter>;

    /// Factory that can create appropriate axial segment pair filters from associated names.
    class AxialSegmentPairFilterFactory : public FilterFactory<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseAxialSegmentPairFilter>;

    public:
      /// Constructor forwarding the default filter name
      explicit AxialSegmentPairFilterFactory(const std::string& defaultFilterName = "realistic");

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseAxialSegmentPairFilter> create(const std::string& filterName) const override;
    };
  }
}
