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

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const svdStateVarNames[] = {
    "seed_cdc_hits",
    "seed_svd_hits",
    "seed_lowest_svd_layer",
    "seed_lowest_cdc_layer",

    "quality_index_triplet",
    "quality_index_circle",
    "quality_index_helix",

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
  class SVDStateVarNames : public TrackFindingCDC::VarNames<BaseSVDStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(svdStateVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return svdStateVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class SVDStateVarSet : public TrackFindingCDC::VarSet<SVDStateVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC object.
    virtual bool extract(const BaseSVDStateFilter::Object* object) override;

  private:
    /// Instance of the circle-fit class
    QualityEstimatorCircleFit m_qualityCircle;
    /// Instance of the Riemann helix-fit class
    QualityEstimatorRiemannHelixFit m_qualityHelix;
    /// Instance of the triplet-fit class
    QualityEstimatorTripletFit m_qualityTriplet;
  };
}
