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

#include <tracking/trackFindingCDC/filters/facet_facet/BaseFacetNeighborChooser.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/facet_facet/CDCRecoFacetRelationTruthVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitFacets.
    class RecordingFacetNeighborChooser: public RecordingFilter<CDCRecoFacetRelationTruthVarSet> {

    public:
      /// Constructor initialising the RecordingNeighborChooser with standard root file name for this neighborChooser.
      RecordingFacetNeighborChooser() :
        RecordingFilter<CDCRecoFacetRelationTruthVarSet>("CDCRecoFacetRelationTruthRecords.root")
      {;}

    };
  }
}
