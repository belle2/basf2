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

#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Specialisation the filter factory for cdc tracks and stereo hits.
     *
     * It knows about all available filters and their parameters.
     * Can collaborate with a Module and expose these parameters to the user in steering files.
     */
    class StereoHitFilterFactory : public FilterFactory<BaseStereoHitFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseStereoHitFilter>;

    public:
      /** Fill the default filter name and parameter values*/
      StereoHitFilterFactory(const std::string& defaultFilterName = "all") : Super(defaultFilterName) { }

      /// Cope the create function from the parent class.
      using Super::create;

      /** Getter for a descriptive purpose of the filter.*/
      virtual std::string getFilterPurpose() const override
      {
        return "Stereo hit to track adding.";
      }

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseStereoHitFilter>
      create(const std::string& filterName) const override;

      /** Getter for the valid filter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for a short identifier of the factory.*/
      virtual std::string getIdentifier() const override
      {
        return "StereoHit";
      }
    };

  }
}
