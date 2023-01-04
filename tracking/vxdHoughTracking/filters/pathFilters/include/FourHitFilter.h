/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/pathFilters/BasePathFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/FourHitVariables.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Filter for four hits.
    /// Basic working principle: use FourHitVariables and ThreeHitVariables and provide four (three B2Vector3D) to each variable.
    /// These are oHit (outer hit), coHit (middle outer hit), ciHit (middle inner hit) and iHit (inner hit) and then calculate
    /// the variables specified in FourHitVariables and ThreeHitVariables for the four positions
    class FourHitFilter : public BasePathFilter {
    public:
      /// Return the weight based on the ThreeHitVariables.
      /// Returns NAN if m_fourHitVariables.getCircleRadiusDifference() of the hit triplet is larger than m_CircleRadiusDifferenceCut
      /// Returns NAN if m_fourHitVariables.getCircleCenterPositionDifference() of the hit triplet is larger than m_CircleCenterPositionDifferenceCut
      /// Returns 1/m_fourHitVariables.getCircleRadiusDifference() else
      TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
      /// Expose the parameters.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// set BField value for estimator
      void beginRun() override;

    private:
      /// Cut on difference of the two circle radii estimated from two triplets
      double m_CircleRadiusDifferenceCut = 10.;
      /// Cut on difference of the two circle center estimated from two triplets
      double m_CircleCenterPositionDifferenceCut = 10.;

      /// Construct empty ThreeHitVariables instance
      FourHitVariables m_fourHitVariables;
    };

  }
}
