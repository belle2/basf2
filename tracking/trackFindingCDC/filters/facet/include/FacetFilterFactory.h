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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     *  Factory that can creates apropriate facet filter instance from parameters.
     *
     *  It knows about all available filters and their parameters.
     *  Can collaborate with a Module and expose these parameters to the user in steering files.
     */
    class FacetFilterFactory :
      public FilterFactory<Filter<CDCFacet> > {

    private:
      /// Type of the base class
      using Super = FilterFactory<Filter<CDCFacet> >;

    public:
      /** Fill the default filter name and parameter values*/
      FacetFilterFactory(const std::string& defaultFilterName = "chi2");

      using Super::create;

      /** Create a cluster filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<Filter<CDCFacet>>
                                             create(const std::string& filterName) const override;

      /** Getter for a descriptive purpose of the clusterfilter.*/
      virtual std::string getFilterPurpose() const override;

      /** Getter for the valid clusterf ilter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override;

    };
  }
}
