/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/states/BasePXDStateFilter.h>
#include <tracking/ckf/pxd/utilities/PXDKalmanStepper.h>

namespace Belle2 {
  /// A very simple filter for all space points.
  class SimplePXDStateFilter : public BasePXDStateFilter {
  public:
    /// Set the cached B field
    void beginRun() final;

    /// default constructor to silence the cppcheck warnings
    SimplePXDStateFilter() = default;

    TrackFindingCDC::Weight operator()(const BasePXDStateFilter::Object& pair) final;

  private:
    /// Cache for the B field at the IP
    double m_cachedBField;

    /// Kalman stepper (CKF) for PXD
    PXDKalmanStepper m_kalmanStepper;

    /// Shortcut for a 2x3 array.
    using MaximalValueArray = double[2][3];

    /// Maximum distance calculated with helix extrapolation in filter 1. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumHelixDistanceXY = {
      {0.295060, 0.295060, 0.295060},
      {0.295060, 0.295060, 0.295060}
    };

    /// Maximum distance calculated with normal extrapolation in filter 2. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumResidual = {
      {0.679690, 0.677690, 0.427060},
      {1.006930, 0.926120, 0.687030}
    };

    /// Maximum chi^2 in filter 3. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumChi2 = {
      {10, 50, 125},
      {20, 60, 85}
    };
  };
}
