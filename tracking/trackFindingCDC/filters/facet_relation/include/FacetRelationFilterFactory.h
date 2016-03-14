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

#include <tracking/trackFindingCDC/filters/facet_relation/BaseFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
       Factory that can creates apropriate facet relation filter instance from parameters.

       It knows about all available filters and their parameters.
       Can collaborate with a Module and expose these parameters to the user in steering files.
    */
    template<>
    class FilterFactory<Filter<Relation<CDCFacet> > > :
      public FilterFactoryBase<Filter<Relation<CDCFacet> > > {

    private:
      /// Type of the base class
      typedef FilterFactoryBase<Filter<Relation<CDCFacet> > > Super;

    public:
      /** Fill the default filter name and parameter values*/
      FilterFactory(const std::string& defaultFilterName = "simple");

      using Super::create;

      /** Create a cluster filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<Filter<Relation<CDCFacet> > > create(const std::string& name) const override;

      /** Getter for a descriptive purpose of the clusterfilter.*/
      virtual std::string getFilterPurpose() const override;

      /** Getter for the valid clusterf ilter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override;

    };

    /// Legacy alias for the facet relation filter factory
    typedef FilterFactory<Filter<Relation<CDCFacet> > > FacetRelationFilterFactory;

  }
}
