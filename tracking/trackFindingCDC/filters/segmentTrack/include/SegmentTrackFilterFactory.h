/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTrack/BaseSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseSegmentTrackFilter>;

    /// Factory that can create appropriate segment to track combinations filters from associated names.
    class SegmentTrackFilterFactory : public FilterFactory<BaseSegmentTrackFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseSegmentTrackFilter>;

    public:
      /// Constructor forwarding the default filter name
      SegmentTrackFilterFactory(const std::string& defaultFilterName = "mva");

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseSegmentTrackFilter> create(const std::string& filterName) const override;
    };
  }
}
