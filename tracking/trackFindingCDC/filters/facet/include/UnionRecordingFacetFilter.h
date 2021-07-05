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

#include <tracking/trackFindingCDC/filters/facet/FacetFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Filter to record multiple chooseable variable sets for facets
    class UnionRecordingFacetFilter: public UnionRecordingFilter<FacetFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<FacetFilterFactory>;

    public:
      /// Get the valid names of variable sets for facets.
      std::vector<std::string> getValidVarSetNames() const final;

      /// Create a concrete variables set for facets from a name.
      std::unique_ptr<BaseVarSet<const CDCFacet>> createVarSet(const std::string& name) const final;
    };
  }
}
