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

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/CDCFacetRelationTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/CDCFacetRelationBasicVarSet.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered facet relations.
    using  RecordingFacetRelationFilter =
      RecordingFilter<VariadicUnionVarSet<CDCFacetRelationBasicVarSet,
      CDCFacetRelationTruthVarSet> >;

  }
}
