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

#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Specialisation the filter factory for segment pairs
     *
     * It knows about all available filters and their parameters.
     * Can collaborate with a Module and expose these parameters to the user in steering files.
     */
    class SegmentTrackFilterFactory : public FilterFactory<BaseSegmentTrackFilter> {

    private:
      /// Type of the base class
      typedef FilterFactory<BaseSegmentTrackFilter> Super;

    public:
      /** Fill the default filter name and parameter values*/
      SegmentTrackFilterFactory(const std::string& defaultFilterName = "simple");

      using Super::create;

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseSegmentTrackFilter> create(const std::string& name) const override;

      /** Getter for a descriptive purpose of the filter.*/
      virtual std::string getFilterPurpose() const override;

      /** Getter for the valid filter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override;

    };
  }
}
