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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcStateBasicVarNames[] = {
    "eventNumber",
    "i_hit",
    "firstHit",
    "iCLayer_lastState",
    "iCLayer",
    "arcLength",
    "hitDistance",
    "seed_theta",
    "seed_r",
    "seed_z",
    "seed_x",
    "seed_y",
    "seed_p",
    "seed_pt",
    "seed_pz",
    "seed_px",
    "seed_py",
    "seed_charge",
    "track_momTheta",
    "track_p",
    "track_pt",
    "track_pz",
    "track_px",
    "track_py",
    "track_posTheta",
    "track_r",
    "track_z",
    "track_x",
    "track_y",
    "track_charge",
    "track_pt_firstHit",
    "track_pz_firstHit",
    "track_r_firstHit",
    "track_z_firstHit",
    "wire_r",
    "wire_z",
    "wire_x",
    "wire_y"
  };

  /// Vehicle class to transport the variable names
  class CDCStateBasicVarNames : public TrackFindingCDC::VarNames<BaseCDCStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcStateBasicVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcStateBasicVarNames[iName];
    }
  };

  /**
   * Var set to store basic quantities related to CDC CKF
   */
  class CDCStateBasicVarSet : public TrackFindingCDC::VarSet<CDCStateBasicVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCStateFilter::Object* result) override;

  private:
    /// use this to get the eventNumber
    StoreObjPtr<EventMetaData> m_eventMetaData;
  };
}
