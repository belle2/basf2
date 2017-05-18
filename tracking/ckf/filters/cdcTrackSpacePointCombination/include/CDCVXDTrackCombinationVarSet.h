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

#include <tracking/ckf/filters/cdcTrackSpacePointCombination/BaseCDCTrackSpacePointCombinationFilter.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointAdvanceAlgorithm.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointKalmanUpdateFitter.h>
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
    "number_of_holes"
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
    /// Generate and assign the variables from the object.
    bool extract(const BaseCDCVXDTrackCombinationFilter::Object* object) final;

  private:
    SpacePointAdvanceAlgorithm m_advanceAlgorithm;
    SpacePointKalmanUpdateFitter m_kalmanAlgorithm;
  };
}
