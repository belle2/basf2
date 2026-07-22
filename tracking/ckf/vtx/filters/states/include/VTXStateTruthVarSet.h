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

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/ckf/vtx/filters/states/BaseVTXStateFilter.h>
#include <tracking/ckf/vtx/utilities/VTXMCUtil.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const vtxStateTruthVarNames[] = {
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
  class VTXStateTruthVarNames : public TrackingUtilities::VarNames<BaseVTXStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackingUtilities::size(vtxStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return vtxStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class VTXStateTruthVarSet : public TrackingUtilities::VarSet<VTXStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseVTXStateFilter::Object* result) override;

  private:
    /// Pointer to the object containing event number etc
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /// MC information used in the VTX CKF
    MCUtil m_mcUtil;
  };
}
