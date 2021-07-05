/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/MVAFeasibleSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/BasicSegmentRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/FitlessSegmentRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/FitSegmentRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the constuction of segment pairs.
    class MVARealisticSegmentRelationFilter : public MVA<BaseSegmentRelationFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseSegmentRelationFilter>;

      /// Type of the VarSet the filter is working on
      using VarSet = VariadicUnionVarSet<BasicSegmentRelationVarSet,
            FitlessSegmentRelationVarSet,
            FitSegmentRelationVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticSegmentRelationFilter();

      /// Tell Root to look at this operator
      using Super::operator();

    private:
      /// Function to object for its signalness
      Weight operator()(const Relation<const CDCSegment2D>& segmentRelation) override;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleSegmentRelationFilter m_feasibleSegmentRelationFilter;
    };
  }
}
