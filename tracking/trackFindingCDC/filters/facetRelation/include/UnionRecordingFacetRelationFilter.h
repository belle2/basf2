/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
