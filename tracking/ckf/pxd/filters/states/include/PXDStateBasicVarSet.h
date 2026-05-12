/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackingUtilities/varsets/VarSet.h>
#include <tracking/trackingUtilities/varsets/VarNames.h>
#include <tracking/trackingUtilities/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/pxd/filters/states/BasePXDStateFilter.h>
#include <tracking/ckf/pxd/utilities/PXDKalmanStepper.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const pxdStateBasicVarNames[] = {
    "distance",
    "xy_distance",
    "z_distance",

    "mSoP_distance",
    "mSoP_xy_distance",
    "mSoP_z_distance",

    "same_hemisphere",

    "layer",
    "number",

    "arcLengthOfHitPosition",
    "arcLengthOfCenterPosition",

    "pt",
    "tan_lambda",
    "phi",

    "sigma_uv",
    "residual_over_sigma",

    "residual",

    "chi2",

    "cluster_charge",
    "cluster_seed_charge",
    "cluster_size",
    "cluster_size_u",
    "cluster_size_v",

    "ladder",
    "sensor",
    "segment",
    "id",

    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",

    "last_cluster_charge",
    "last_cluster_seed_charge",
    "last_cluster_size",
    "last_cluster_size_u",
    "last_cluster_size_v",
  };

  /// Vehicle class to transport the variable names
  class PXDStateBasicVarNames : public TrackingUtilities::VarNames<BasePXDStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackingUtilities::size(pxdStateBasicVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return pxdStateBasicVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class PXDStateBasicVarSet : public TrackingUtilities::VarSet<PXDStateBasicVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    virtual bool extract(const BasePXDStateFilter::Object* pair) override;

  private:
    /// Kalmap update filter used in this var set
    PXDKalmanStepper m_kalmanStepper;
  };
}
