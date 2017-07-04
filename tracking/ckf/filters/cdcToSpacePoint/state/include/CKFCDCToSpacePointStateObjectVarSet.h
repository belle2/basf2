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

#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateObjectFilter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfCDCToSpacePointStateObjectVarNames[] = {
    "track_position_x",
    "track_position_y",
    "track_position_z",

    "hit_position_x",
    "hit_position_y",
    "hit_position_z",

    "track_position_at_hit_x",
    "track_position_at_hit_y",
    "track_position_at_hit_z",

    "ladder",
    "sensor",
    "segment",
    "id",

    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",

    "pt",
    "tan_lambda",
    "phi",

    "C_00",
    "C_01",
    "C_02",
    "C_03",
    "C_04",
    "C_05",
    "C_10",
    "C_11",
    "C_12",
    "C_13",
    "C_14",
    "C_15",
    "C_20",
    "C_21",
    "C_22",
    "C_23",
    "C_24",
    "C_25",
    "C_30",
    "C_31",
    "C_32",
    "C_33",
    "C_34",
    "C_35",
    "C_40",
    "C_41",
    "C_42",
    "C_43",
    "C_44",
    "C_45",
    "C_50",
    "C_51",
    "C_52",
    "C_53",
    "C_54",
    "C_55",

    "C5_00",
    "C5_01",
    "C5_02",
    "C5_03",
    "C5_04",
    "C5_10",
    "C5_11",
    "C5_12",
    "C5_13",
    "C5_14",
    "C5_20",
    "C5_21",
    "C5_22",
    "C5_23",
    "C5_24",
    "C5_30",
    "C5_31",
    "C5_32",
    "C5_33",
    "C5_34",
    "C5_40",
    "C5_41",
    "C5_42",
    "C5_43",
    "C5_44",
    "C5_45",

    "state_0",
    "state_1",
    "state_2",
    "state_3",
    "state_4",

    "m_0_state",
    "m_0_cov",
    "m_1_state",
    "m_1_cov",

    "is_u_0",
    "is_u_1"
  };

  /// Vehicle class to transport the variable names
  class CKFCDCToSpacePointStateObjectVarNames : public TrackFindingCDC::VarNames<BaseCKFCDCToSpacePointStateObjectFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfCDCToSpacePointStateObjectVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfCDCToSpacePointStateObjectVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class CKFCDCToSpacePointStateObjectVarSet : public TrackFindingCDC::VarSet<CKFCDCToSpacePointStateObjectVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC object.
    virtual bool extract(const BaseCKFCDCToSpacePointStateObjectFilter::Object* object) override;
  };
}
