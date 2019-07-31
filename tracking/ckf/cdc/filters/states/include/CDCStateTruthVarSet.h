/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcStateTruthVarNames[] = {
    "match",
    "PDG"
  };

  /// Vehicle class to transport the variable names
  class CDCStateTruthVarNames : public TrackFindingCDC::VarNames<BaseCDCStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcStateTruthVarNames[iName];
    }
  };

  /**
   * Var set to store basic quantities related to CDC CKF (using truth information)
   */
  class CDCStateTruthVarSet : public TrackFindingCDC::VarSet<CDCStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCStateFilter::Object* result) override;
  };
}
