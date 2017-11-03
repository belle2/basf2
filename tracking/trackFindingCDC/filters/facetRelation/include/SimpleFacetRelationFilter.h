/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCFacet;

    /// Class filtering the neighborhood of facets based on simple criterions.
    class SimpleFacetRelationFilter : public BaseFacetRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetRelationFilter;

    public:
      /// Constructor using default direction of flight deviation cut off.
      SimpleFacetRelationFilter();

      /// Constructor using given direction of flight deviation cut off.
      explicit SimpleFacetRelationFilter(double deviationCosCut);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    public:
      /// Main filter method returning the weight of the neighborhood relation.
      Weight operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet) final;

    private:
      /// Memory for the used direction of flight deviation.
      double m_param_deviationCosCut;
    };
  }
}
