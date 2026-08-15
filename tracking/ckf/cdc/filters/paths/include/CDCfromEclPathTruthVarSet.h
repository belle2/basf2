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
    "seed_py_truth"
  };

  /// Vehicle class to transport the variable names
  class CDCfromEclPathTruthVarNames : public TrackingUtilities::VarNames<BaseCDCPathFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackingUtilities::size(cdcfromEclPathTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcfromEclPathTruthVarNames[iName];
    }
  };

  /**
   * Var set to store basic quantities related to CDC CKF (using truth information)
   */
  class CDCfromEclPathTruthVarSet : public TrackingUtilities::VarSet<CDCfromEclPathTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCPathFilter::Object* path) override;
  };
}
