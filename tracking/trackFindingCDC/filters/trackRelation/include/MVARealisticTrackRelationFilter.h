/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/BasicTrackRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/trackRelation/FitTrackRelationVarSet.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
  }
  namespace TrackFindingCDC {

    /// Final filter for the construction of segment pairs.
    class MVARealisticTrackRelationFilter : public TrackingUtilities::MVA<BaseTrackRelationFilter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::MVA<BaseTrackRelationFilter>;

      /// Set of variables used in this filter
      using VarSet = TrackingUtilities::VariadicUnionVarSet<BasicTrackRelationVarSet, FitTrackRelationVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticTrackRelationFilter();

      /// Function to object for its signalness
      TrackingUtilities::Weight operator()(const TrackingUtilities::Relation<const TrackingUtilities::CDCTrack>& trackRelation) final;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleTrackRelationFilter m_feasibleTrackRelationFilter;
    };
  }
}
