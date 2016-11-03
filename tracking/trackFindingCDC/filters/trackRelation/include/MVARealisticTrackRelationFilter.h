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

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/trackFindingCDC/filters/trackRelation/BasicTrackRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/trackRelation/FitTrackRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the constuction of segment pairs.
    class MVARealisticTrackRelationFilter :
      public MVA<BaseTrackRelationFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseTrackRelationFilter>;

      /// Set of variables used in this filter
      using VarSet = VariadicUnionVarSet <
                     BasicTrackRelationVarSet,
                     FitTrackRelationVarSet >;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticTrackRelationFilter();

      /// Initialize the expert before event processing.
      virtual void initialize() override;

      /// Signal to load new run parameters
      virtual void beginRun() override;

      /// Function to object for its signalness
      virtual Weight operator()(const Relation<const CDCTrack>& trackRelation) override;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleTrackRelationFilter m_feasibleTrackRelationFilter;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
