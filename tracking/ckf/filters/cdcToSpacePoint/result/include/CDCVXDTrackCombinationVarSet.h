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

#include <tracking/ckf/filters/cdcToSpacePoint/result/BaseCDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/findlets/base/AdvanceAlgorithm.h>
#include <tracking/ckf/findlets/base/KalmanUpdateFitter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcVXDTrackCombinationNames[] = {
    "chi2_vxd_full",
    "chi2_vxd_max",
    "chi2_vxd_min",
    "chi2_vxd_mean",
    "number_of_hits",
    "prob",
    "pt",
    "chi2_cdc",
    "number_of_holes",
    "last_hit_layer",
    "first_hit_layer",
    "chi2",
    "has_missing_layer_1",
    "has_missing_layer_2",
    "has_missing_layer_3",
    "has_missing_layer_4",
    "has_missing_layer_5",
    "has_missing_layer_6",
    "number_of_overlap_hits",
    "theta",
    "distance_to_cdc_track",
    "distance_to_cdc_track_xy"
  };

  /// Vehicle class to transport the variable names
  class CDCVXDTrackCombinationVarNames : public
    TrackFindingCDC::VarNames<BaseCDCVXDTrackCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcVXDTrackCombinationNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcVXDTrackCombinationNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CDCVXDTrackCombinationVarSet : public TrackFindingCDC::VarSet<CDCVXDTrackCombinationVarNames> {

  public:
    CDCVXDTrackCombinationVarSet() : TrackFindingCDC::VarSet<CDCVXDTrackCombinationVarNames>()
    {
      ModuleParamList moduleParamList;
      const std::string prefix = "";
      m_advanceAlgorithm.exposeParameters(&moduleParamList, prefix);
      moduleParamList.getParameter<bool>("useMaterialEffects").setDefaultValue(false);
    }

    /// Generate and assign the variables from the object.
    bool extract(const BaseCDCVXDTrackCombinationFilter::Object* object) final;

  private:
    AdvanceAlgorithm m_advanceAlgorithm;
    KalmanUpdateFitter m_kalmanAlgorithm;
  };
}
