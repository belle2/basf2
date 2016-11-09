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

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     *  Factory that creates apropriate segment relation filter instances from parameters.
     *
     *  It knows about all available filters and their parameters.
     *  Can collaborate with a Module and expose these parameters to the user in steering files.
     */
    class SegmentRelationFilterFactory :
      public FilterFactory<BaseSegmentRelationFilter > {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseSegmentRelationFilter >;

    public:
      /** Fill the default filter name and parameter values*/
      SegmentRelationFilterFactory(const std::string& defaultFilterName = "realistic");

      using Super::create;

      /** Create a cluster filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseSegmentRelationFilter>
      create(const std::string& filterName) const override;

      /** Getter for a descriptive purpose of the clusterfilter.*/
      virtual std::string getFilterPurpose() const override;

      /** Getter for the valid clusterf ilter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for a short identifier of the factory.*/
      virtual std::string getIdentifier() const override;

    };

  }
}
