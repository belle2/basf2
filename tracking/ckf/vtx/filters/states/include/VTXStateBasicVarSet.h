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

#include <tracking/ckf/vtx/filters/states/BaseVTXStateFilter.h>
#include <tracking/ckf/vtx/utilities/VTXKalmanStepper.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const vtxStateBasicVarNames[] = {
    "distance",
    "xy_distance",
    "z_distance",

    "mSoP_distance",
    "mSoP_xy_distance",
    "mSoP_z_distance",

    "number",

    "arcLengthOfHitPosition",
    "arcLengthOfCenterPosition",

    "pt",
    "tan_lambda",
    "phi",

    "residual",
    "sigma_uv",
    "residual_over_sigma",

    "chi2",

    "layer",
    "ladder",
    "sensor",
    "segment",
    "id",

    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",

  };

  /// Vehicle class to transport the variable names
  class VTXStateBasicVarNames : public TrackFindingCDC::VarNames<BaseVTXStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(vtxStateBasicVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return vtxStateBasicVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class VTXStateBasicVarSet : public TrackFindingCDC::VarSet<VTXStateBasicVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    virtual bool extract(const BaseVTXStateFilter::Object* pair) override;

  private:
    /// Kalmap update filter used in this var set
    VTXKalmanStepper m_kalmanStepper;
  };
}
