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
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered relations between facets.
    class UnionRecordingFacetRelationFilter: public UnionRecordingFilter<Relation<CDCFacet> > {

    private:
      /// Type of the base class
      typedef  UnionRecordingFilter<Relation<CDCFacet> > Super;

    public:
      /// Valid names of variable sets for facets.
      virtual std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for facets from a name.
      virtual
      std::unique_ptr<BaseVarSet<Relation<CDCFacet> > >
      createVarSet(const std::string& name) const override;
    };
  }
}
