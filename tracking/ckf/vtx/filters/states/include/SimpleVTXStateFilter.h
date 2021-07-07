/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/vtx/filters/states/BaseVTXStateFilter.h>
#include <tracking/ckf/vtx/utilities/VTXKalmanStepper.h>

namespace Belle2 {
  /// A very simple filter for all space points.
  class SimpleVTXStateFilter : public BaseVTXStateFilter {
  public:
    /// Set the cached B field
    void beginRun() final;

    /// default constructor to silence the cppcheck warnings
    SimpleVTXStateFilter() = default;

    TrackFindingCDC::Weight operator()(const BaseVTXStateFilter::Object& pair) final;

  private:
    /// Cache for the B field at the IP
    double m_cachedBField;

    /// Kalman stepper (CKF) for VTX
    VTXKalmanStepper m_kalmanStepper;

    // ATTENTION FIXME: hardcoded 7 layers
    /// Shortcut for a 2x3 array.
    using MaximalValueArray = double[7][3];

    /// Maximum distance calculated with helix extrapolation in filter 1. Numbers calculated on MC.
    // ATTENTION FIXME: Hardcoded values taken from PXD and SVD. Recalculate!!!
    //      For L1 and L2 it really HelixDistanceXY (from PXD), for L3-L6 it's HelixDistance (from SVD)
    static constexpr const MaximalValueArray m_param_maximumHelixDistanceXY = {
      // high pt, middle pt, low pt
      {0.295060, 0.295060, 0.295060}, // layer 1
      {0.295060, 0.295060, 0.295060}, // layer 2
      {1.38532, 1.44346, 1.53386}, // layer 3
      {1.96701, 2.17819, 2.19784}, // layer 4
      {2.33064, 2.67768, 2.67174}, // layer 5
      {6, 6, 6}, // layer 6
      {6, 6, 6}, // layer 7
    };

    /// Maximum distance calculated with normal extrapolation in filter 2. Numbers calculated on MC.
    // ATTENTION FIXME: Hardcoded values taken from PXD and SVD. Recalculate!!!
    static constexpr const MaximalValueArray m_param_maximumResidual = {
      // high pt, middle pt, low pt
      {0.679690, 0.677690, 0.427060}, // layer 1
      {1.006930, 0.926120, 0.687030}, // layer 2
      {1.52328, 1.57714, 1.63069}, // layer 3
      {1.61301, 1.78301, 1.75973}, // layer 4
      {1.85188, 2.19571, 2.38167}, // layer 5
      {5.5, 5, 5}, // layer 6
      {5.5, 5, 5}, // layer 7
    };

    /// Maximum chi^2 in filter 3. Numbers calculated on MC.
    // ATTENTION FIXME: Hardcoded values taken from PXD and SVD. Recalculate!!!
    static constexpr const MaximalValueArray m_param_maximumChi2 = {
      // high pt, middle pt, low pt
      {50, 50, 125}, // layer 1
      {60, 60, 125}, // layer 2
      {70, 70, 125}, // layer 3
      {80, 80, 125}, // layer 4
      {90, 90, 125}, // layer 5
      {100, 100, 125}, // layer 6
      {110, 110, 125}, // layer 7
    };
  };
}
