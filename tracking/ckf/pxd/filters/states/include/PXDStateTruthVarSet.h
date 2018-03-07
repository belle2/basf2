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

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/ckf/pxd/filters/states/BasePXDStateFilter.h>
#include <tracking/ckf/pxd/utilities/PXDMCUtil.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const pxdStateTruthVarNames[] = {
    "truth",
    "truth_inverted",
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
  class PXDStateTruthVarNames : public TrackFindingCDC::VarNames<BasePXDStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(pxdStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return pxdStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class PXDStateTruthVarSet : public TrackFindingCDC::VarSet<PXDStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BasePXDStateFilter::Object* result) override;

  private:
    StoreObjPtr<EventMetaData> m_eventMetaData;

    MCUtil m_mcUtil;
  };
}
