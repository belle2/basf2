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

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/ckf/svd/filters/states/BaseSVDStateFilter.h>
#include <tracking/ckf/svd/utilities/SVDMCUtil.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const svdStateTruthVarNames[] = {
    "truth",
    "truth_position_x",
    "truth_position_y",
    "truth_position_z",
    "truth_momentum_x",
    "truth_momentum_y",
    "truth_momentum_z",
    "truth_event_id",
    "truth_seed_number"
  };

  /// Vehicle class to transport the variable names
  class SVDStateTruthVarNames : public TrackFindingCDC::VarNames<BaseSVDStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(svdStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return svdStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class SVDStateTruthVarSet : public TrackFindingCDC::VarSet<SVDStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseSVDStateFilter::Object* result) override;

  private:
    /// Pointer to the object containing event number etc
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /// MC information used in the SVD CKF
    MCUtil m_mcUtil;
  };
}
