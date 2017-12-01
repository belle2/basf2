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

#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Filter to record multiple chooseable variable sets for facet relations
    class UnionRecordingFacetRelationFilter: public UnionRecordingFilter<FacetRelationFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<FacetRelationFilterFactory>;

    public:
      /// Get the valid names of variable sets for facet relations.
      std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for facet relations from a name.
      std::unique_ptr<BaseVarSet<Relation<const CDCFacet> > >
      createVarSet(const std::string& name) const override;
    };
  }
}
