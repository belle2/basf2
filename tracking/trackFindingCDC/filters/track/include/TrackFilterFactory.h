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

#include <tracking/trackFindingCDC/filters/track/TrackFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Specialisation the filter factory for cdc tracks
     *
     * It knows about all available filters and their parameters.
     * Can collaborate with a Module and expose these parameters to the user in steering files.
     */
    class TrackFilterFactory : public FilterFactory<BaseTrackFilter> {

    private:
      /// Type of the base class
      typedef FilterFactory<BaseTrackFilter> Super;

    public:
      /** Fill the default filter name and parameter values*/
      TrackFilterFactory(const std::string& defaultFilterName = "simple") : Super(defaultFilterName) { }

      using Super::create;

      /** Getter for a descriptive purpose of the filter.*/
      virtual std::string getFilterPurpose() const override
      {
        return "Segment background finder.";
      }

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseTrackFilter> create(const std::string& name) const override;

      /** Getter for the valid filter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override
      {
        return "Track";
      }
    };

  }
}
