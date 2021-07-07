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

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const vtxStateVarNames[] = {
    "seed_cdc_hits",
    "seed_lowest_cdc_layer",
//     "seed_vtx_hits",
//     "seed_lowest_vtx_layer",

    "quality_index_triplet",
    "quality_index_circle",
    "quality_index_helix",

    "cluster_charge",
//     "mean_rest_cluster_charge",
//     "min_rest_cluster_charge",
//     "std_rest_cluster_charge",

    "cluster_seed_charge",
//     "mean_rest_cluster_seed_charge",
//     "min_rest_cluster_seed_charge",
//     "std_rest_cluster_seed_charge",

    "cluster_size",
//     "mean_rest_cluster_size",
//     "min_rest_cluster_size",
//     "std_rest_cluster_size",

    // TODO: as long there are no SNR values available, these values can't be used
    //       include them when SNR for the VTX is available
    // "cluster_snr",
    // "mean_rest_cluster_snr",
    // "min_rest_cluster_snr",
    // "std_rest_cluster_snr",

    "cluster_charge_over_size",
//     "mean_rest_cluster_charge_over_size",
//     "min_rest_cluster_charge_over_size",
//     "std_rest_cluster_charge_over_size",
  };

  /// Vehicle class to transport the variable names
  class VTXStateVarNames : public TrackFindingCDC::VarNames<BaseVTXStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(vtxStateVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return vtxStateVarNames[iName];
    }
  };

  /**
   * Var set used in the VTX-CDC-Merger for calculating the probability of a VTX-CDC-track match.
   */
  class VTXStateVarSet : public TrackFindingCDC::VarSet<VTXStateVarNames> {

  public:
    /// Generate and assign the variables from the VTX-CDC object.
    virtual bool extract(const BaseVTXStateFilter::Object* object) override;

  private:
    /// Instance of the circle-fit class
    QualityEstimatorCircleFit m_qualityCircle;
    /// Instance of the Riemann helix-fit class
    QualityEstimatorRiemannHelixFit m_qualityHelix;
    /// Instance of the triplet-fit class
    QualityEstimatorTripletFit m_qualityTriplet;
  };
}
