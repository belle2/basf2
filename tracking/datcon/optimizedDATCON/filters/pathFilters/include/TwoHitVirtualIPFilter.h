/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/filters/pathFilters/BasePathFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ThreeHitVariables.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

namespace Belle2 {
  /// Filter for two hits plus a virtual IP.
  /// Basic working principle: use ThreeHitVariables and provide three B2Vector3D to each variable.
  /// These are oHit (outer hit), cHit (middle hit), and iHit (inner hit), which is the virtual IP, and then calculate
  /// the variables specified in ThreeHitVariables using the three positions.
  class TwoHitVirtualIPFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// set BField value for estimator
    void beginRun() override;

  private:
    /// virtual IP vector
    B2Vector3D m_virtualIPPosition;
    /// cut for cosine in RZ between the two vectors (oHit - cHit) and (cHit - iHit)
    double m_param_cosRZCut = 0.95;
    /// cut on the difference between circle radius and circle center position in the x-y plane
    /// to check if the track is compatible with passing through the IP
    double m_param_circleIPDistanceCut = 2.0;

    /// Construct empty ThreeHitVariables instance
    ThreeHitVariables m_threeHitVariables;

    /// BeamSpot from DB
    DBObjPtr<BeamSpot> m_BeamSpotDB;
    /// Actual BeamSpot
    BeamSpot m_BeamSpot;
  };
}
