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

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Factory that can create appropriate axial segment pair filters from associated names.
    class AxialSegmentPairFilterFactory : public FilterFactory<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseAxialSegmentPairFilter>;

    public:
      /// Constructor forwarding the default filter name
      AxialSegmentPairFilterFactory(const std::string& defaultFilterName = "simple");

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
