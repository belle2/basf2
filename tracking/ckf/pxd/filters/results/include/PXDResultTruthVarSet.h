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
#include <tracking/trackingUtilities/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/pxd/utilities/PXDMCUtil.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  class CKFToPXDResult;
  class EventMetaData;

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
  class PXDResultTruthVarNames : public TrackingUtilities::VarNames<CKFToPXDResult> {

  public:
    /// Number of variables to be generated.
    // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
    // at least tell cppcheck that everything is fine
    // cppcheck-suppress duplInheritedMember
    static const size_t nVars = TrackingUtilities::size(pxdResultTruthNames);

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
  class PXDResultTruthVarSet : public TrackingUtilities::VarSet<PXDResultTruthVarNames> {
  public:
    /// Generate and assign the variables from the object.
    bool extract(const CKFToPXDResult* result) final;

  private:
    /// Pointer to the object containing event number etc
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /// MC information used in the PXD CKF
    MCUtil m_mcUtil;
  };
}
