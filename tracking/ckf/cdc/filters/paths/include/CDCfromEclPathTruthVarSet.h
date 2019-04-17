/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/ckf/cdc/filters/paths/BaseCDCPathFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcfromEclPathTruthVarNames[] = {
    "matched",
    "daughters",
    "PDG",
    "mcTrackHits",
    "seed_p_truth",
    "seed_theta_truth",
    "seed_pt_truth",
    "seed_pz_truth",
    "seed_px_truth",
    "seed_py_truth",
    "mcTrackEnd_p",
    "mcTrackEnd_pt",
    "mcTrackEnd_pz",
    "mcTrackEnd_px",
    "mcTrackEnd_py",
    "mcTrackEnd_z",
    "mcTrackEnd_x",
    "mcTrackEnd_y",
    "mcTrackEnd_posTheta",
    "mcTrackEnd_momTheta"
  };

  /// Vehicle class to transport the variable names
  class CDCfromEclPathTruthVarNames : public TrackFindingCDC::VarNames<BaseCDCPathFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcfromEclPathTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcfromEclPathTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CDCfromEclPathTruthVarSet : public TrackFindingCDC::VarSet<CDCfromEclPathTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCPathFilter::Object* path) override;
  };
}
