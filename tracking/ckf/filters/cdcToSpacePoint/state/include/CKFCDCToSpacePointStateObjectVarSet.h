/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateObjectFilter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfCDCToSpacePointStateObjectVarNames[] = {
    "ladder",
    "sensor",
    "segment",
    "id",

    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",

    "pt",
    "tan_lambda",
    "phi",

    "cluster_1_charge",
    "cluster_2_charge",
    "mean_rest_cluster_charge",
    "min_rest_cluster_charge",
    "std_rest_cluster_charge",

    "cluster_1_seed_charge",
    "cluster_2_seed_charge",
    "mean_rest_cluster_seed_charge",
    "min_rest_cluster_seed_charge",
    "std_rest_cluster_seed_charge",

    "cluster_1_size",
    "cluster_2_size",
    "mean_rest_cluster_size",
    "min_rest_cluster_size",
    "std_rest_cluster_size",

    "cluster_1_snr",
    "cluster_2_snr",
    "mean_rest_cluster_snr",
    "min_rest_cluster_snr",
    "std_rest_cluster_snr",

    "cluster_1_charge_over_size",
    "cluster_2_charge_over_size",
    "mean_rest_cluster_charge_over_size",
    "min_rest_cluster_charge_over_size",
    "std_rest_cluster_charge_over_size",
  };

  /// Vehicle class to transport the variable names
  class CKFCDCToSpacePointStateObjectVarNames : public TrackFindingCDC::VarNames<BaseCKFCDCToSpacePointStateObjectFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfCDCToSpacePointStateObjectVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfCDCToSpacePointStateObjectVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class CKFCDCToSpacePointStateObjectVarSet : public TrackFindingCDC::VarSet<CKFCDCToSpacePointStateObjectVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC object.
    virtual bool extract(const BaseCKFCDCToSpacePointStateObjectFilter::Object* object) override;
  };
}
