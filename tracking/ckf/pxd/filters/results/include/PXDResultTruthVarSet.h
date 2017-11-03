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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>
#include <tracking/ckf/pxd/utilities/PXDMCUtil.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const pxdResultTruthNames[] = {
    "truth",
    "truth_number_of_correct_hits",
    "truth_number_of_mc_pxd_hits",
    "truth_number_of_mc_svd_hits",
    "truth_number_of_mc_cdc_hits",
    "truth_event_number",
    "truth_seed_number",
  };

  /// Vehicle class to transport the variable names
  class PXDResultTruthVarNames : public TrackFindingCDC::VarNames<CKFToPXDResult> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(pxdResultTruthNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return pxdResultTruthNames[iName];
    }
  };

  /**
   * Var set used in the CKF for calculating the probability of a correct result,
   * which knows the truth information if two tracks belong together or not.
   */
  class PXDResultTruthVarSet : public TrackFindingCDC::VarSet<PXDResultTruthVarNames> {
  public:
    /// Generate and assign the variables from the object.
    bool extract(const CKFToPXDResult* result) final;

  private:
    StoreObjPtr<EventMetaData> m_eventMetaData;

    MCUtil m_mcUtil;
  };
}
