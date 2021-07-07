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

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcfromEclStateTruthVarNames[] = {
    "match",
    "PDG",
    "seed_p_truth",
    "seed_theta_truth",
    "seed_pt_truth",
    "seed_pz_truth",
    "seed_px_truth",
    "seed_py_truth"
  };

  /// Vehicle class to transport the variable names
  class CDCfromEclStateTruthVarNames : public TrackFindingCDC::VarNames<BaseCDCStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcfromEclStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcfromEclStateTruthVarNames[iName];
    }
  };

  /**
   * Var set to store basic quantities related to CDC CKF (using truth information)
   */
  class CDCfromEclStateTruthVarSet : public TrackFindingCDC::VarSet<CDCfromEclStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCStateFilter::Object* result) override;
  };
}
