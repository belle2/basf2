/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
