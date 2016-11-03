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

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the constuction of segment pairs.
    class MVARealisticSegmentPairFilter :
      public MVAFilter<VariadicUnionVarSet<
      BasicSegmentPairVarSet,
      FitlessSegmentPairVarSet,
      FitSegmentPairVarSet> > {

    private:
      /// Type of the base class
      using Super = MVAFilter<VariadicUnionVarSet<
                    BasicSegmentPairVarSet,
                    FitlessSegmentPairVarSet,
                    FitSegmentPairVarSet> >;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticSegmentPairFilter()
        : Super("trackfindingcdc_RealisticSegmentPairFilter", 0.02)
      {
      }

      /// Initialize the expert before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        m_feasibleSegmentPairFilter.initialize();
      }

      /// Signal to load new run parameters
      virtual void beginRun() override
      {
        Super::beginRun();
        m_feasibleSegmentPairFilter.beginRun();
      }

      /// Function to object for its signalness
      virtual Weight operator()(const CDCSegmentPair& segmentPair) override
      {
        double isFeasibleWeight = m_feasibleSegmentPairFilter(segmentPair);
        if (std::isnan(isFeasibleWeight)) {
          return NAN;
        } else {
          return Super::operator()(segmentPair);
        }
      }

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleSegmentPairFilter m_feasibleSegmentPairFilter;

    };

  }
}
