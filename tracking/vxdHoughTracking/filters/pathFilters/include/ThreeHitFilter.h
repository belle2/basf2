/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/pathFilters/BasePathFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/ThreeHitVariables.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Filter for three hits.
    /// Basic working principle: use ThreeHitVariables and provide three B2Vector3D to each variable.
    /// These are oHit (outer hit), cHit (middle hit), and iHit (inner hit) and then calculate
    /// the variables specified in ThreeHitVariables using the three positions, often using the difference
    /// (oHit - cHit) and (cHit - iHit).
    class ThreeHitFilter : public BasePathFilter {

    public:
      /// Return the weight based on the ThreeHitVariables.
      /// Returns NAN if m_threeHitVariables.getCosAngleRZSimple() of the hit triplet is smaller than m_cosRZCut
      /// Returns NAN if m_threeHitVariables.getCircleDistanceIP() of the hit triplet is larger than m_circleIPDistanceCut
      /// Returns 1/m_threeHitVariables.getCircleDistanceIP() else
      TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
      /// Expose the parameters.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// set BField value for estimator
      void beginRun() override;

    private:
      /// Construct empty ThreeHitVariables instance
      ThreeHitVariables m_threeHitVariables;

      /// cut for cosine in RZ between the two vectors (oHit - cHit) and (cHit - iHit)
      double m_cosRZCut = 0.95;
      /// cut on the difference between circle radius and circle center position in the x-y plane
      /// to check if the track is compatible with passing through the IP
      double m_circleIPDistanceCut = 2.0;
    };

  }
}
