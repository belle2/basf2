/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/svd/filters/states/BaseSVDStateFilter.h>
#include <tracking/ckf/svd/utilities/SVDKalmanStepper.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const svdStateBasicVarNames[] = {
    "distance",
    "xy_distance",
    "z_distance",

    "mSoP_distance",
    "mSoP_xy_distance",
    "mSoP_z_distance",

    "same_hemisphere",

    "layer",
    "number",
    "ladder",
    "sensor",
    "segment",
    "id",

    "cluster_1_time",
    "cluster_2_time",

    "arcLengthOfHitPosition",
    "arcLengthOfCenterPosition",

    "pt",
    "tan_lambda",
    "phi",

    "sigma_uv",
    "residual_over_sigma",

    "residual",

    "chi2",

    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",

    "last_cluster_1_time",
    "last_cluster_2_time",
  };

  /// Vehicle class to transport the variable names
  class SVDStateBasicVarNames : public TrackFindingCDC::VarNames<BaseSVDStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(svdStateBasicVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return svdStateBasicVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class SVDStateBasicVarSet : public TrackFindingCDC::VarSet<SVDStateBasicVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    virtual bool extract(const BaseSVDStateFilter::Object* pair) override;

  private:
    /// Kalman stepper (CKF) for SVD
    SVDKalmanStepper m_kalmanStepper;
  };
}
