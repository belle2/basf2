/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/BasicTrackRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/trackRelation/HitGapTrackRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of track pairs based on simple criteria without the common fit.
    class MVAFeasibleTrackRelationFilter : public MVA<BaseTrackRelationFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseTrackRelationFilter>;

      /// Type of the VarSet the filter is working on
      using VarSet = VariadicUnionVarSet<BasicTrackRelationVarSet, HitGapTrackRelationVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFeasibleTrackRelationFilter();
    };
  }
}
