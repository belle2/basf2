/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

#include <map>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseSegmentPairFilter>;

    /// Factory that can create appropriate segment pair filters from associated names.
    class SegmentPairFilterFactory : public FilterFactory<BaseSegmentPairFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseSegmentPairFilter>;

    public:
      /// Constructor forwarding the default filter name
      explicit SegmentPairFilterFactory(const std::string& defaultFilterName = "realistic");

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const final;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const final;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const final;

      /// Create a filter with the given name.
      std::unique_ptr<BaseSegmentPairFilter> create(const std::string& filterName) const final;
    };
  }
}
