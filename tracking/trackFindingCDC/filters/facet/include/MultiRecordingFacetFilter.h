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
#include <tracking/trackFindingCDC/filters/base/MultiRecordingFilter.h>
#include <tracking/trackFindingCDC/filters/facet/CDCFacetVarSets.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitFacets.
    class MultiRecordingFacetFilter: public MultiRecordingFilter<CDCFacet> {

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      MultiRecordingFacetFilter() :
        MultiRecordingFilter<CDCFacet>("CDCFacetTruthRecords.root")
      {;}

      /// Valid names of variable sets for facets.
      virtual std::vector<std::string> getValidVarSetNames() const override
      { return {"fitless", "fit", "truth"}; }

      /// Create a concrete variables set for facets from a name.
      virtual
      std::unique_ptr<BaseVarSet<Object>> createVarSet(const std::string& name) const override
      {
        if (name == "fitless") {
          return std::unique_ptr<BaseVarSet<CDCFacet> >(new CDCFacetFitlessVarSet());
        } else if (name == "fit") {
          return std::unique_ptr<BaseVarSet<CDCFacet> >(new CDCFacetFitVarSet());
        } else if (name == "truth") {
          return std::unique_ptr<BaseVarSet<CDCFacet> >(new CDCFacetTruthVarSet());
        } else {
          return std::unique_ptr<BaseVarSet<CDCFacet> >(nullptr);
        }
      }
    };
  }
}
