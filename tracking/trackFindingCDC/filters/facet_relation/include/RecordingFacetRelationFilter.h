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
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/CDCFacetRelationTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/facet_relation/CDCFacetRelationBasicVarSet.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitFacets.
    class RecordingFacetRelationFilter: public RecordingFilter<VariadicUnionVarSet<
      CDCFacetRelationBasicVarSet,
      CDCFacetRelationTruthVarSet> > {

    public:
      /// Constructor initialising the RecordingRelationFilter with standard root file name.
      RecordingFacetRelationFilter() :
        RecordingFilter<VariadicUnionVarSet<
        CDCFacetRelationBasicVarSet,
        CDCFacetRelationTruthVarSet> >("CDCFacetRelationTruthRecords.root")
      {;}

    };
  }
}
