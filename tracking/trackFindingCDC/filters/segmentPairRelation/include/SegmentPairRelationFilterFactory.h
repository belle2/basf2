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

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
       Factory that can creates apropriate segment pair relation filter instance from parameters.

       It knows about all available filters and their parameters.
       Can collaborate with a Module and expose these parameters to the user in steering files.
    */
    class SegmentPairRelationFilterFactory : public FilterFactory<BaseSegmentPairRelationFilter> {

    public:
      /// Type of the base class
      using Super = FilterFactory<BaseSegmentPairRelationFilter>;

      /** Fill the default filter name and parameter values*/
      SegmentPairRelationFilterFactory(const std::string& defaultFilterName = "simple");

      using FilterFactory<BaseSegmentPairRelationFilter>::create;

      /** Create a cluster filter with the given name, does not set filter specific parameters. */
      virtual
      std::unique_ptr<BaseSegmentPairRelationFilter> create(const std::string& name) const override;

      /** Getter for a descriptive purpose of the clusterfilter.*/
      virtual std::string getFilterPurpose() const override;

      /** Getter for the valid clusterf ilter names and a description for each */
      virtual
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override;

    };
  }
}
