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

#include <tracking/ckf/cdc/filters/paths/BaseCDCPathFilter.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcPathBasicVarNames[] = {
    "eventNumber",
    "totalHits",
    "reachedEnd",
    "totalHitsSeedTrack",
    "firstChi2",
    "lastChi2",
    "seedICLayer",
    "firstICLayer",
    "lastICLayer",
    "seed_r",
    "seed_z",
    "seed_x",
    "seed_y",
    "seed_p",
    "seed_theta",
    "seed_pt",
    "seed_pz",
    "seed_px",
    "seed_py",
    "seed_charge",
    "track_theta",
    "track_p",
    "track_pt",
    "track_pz",
    "track_px",
    "track_py",
    "track_charge",
    "chargeFlip",
    "ICLayerLast0",
    "ICLayerLast1",
    "ICLayerLast2",
    "IWireLast0",
    "IWireLast1",
    "IWireLast2",
    "flipPos0",
    "flipPos1",
    "flipPos2",
    "flipPos3",
    "arcLength0",
    "arcLength1",
    "arcLength2",
    "arcLength3",
    "arcLength4",
    "arcLength5",
    "arcLength6",
    "arcLength7",
    "arcLength8",
    "arcLength9",
    "hitDistance0",
    "hitDistance1",
    "hitDistance2",
    "hitDistance3",
    "hitDistance4",
    "hitDistance5",
    "hitDistance6",
    "hitDistance7",
    "hitDistance8",
    "hitDistance9"
  };

  /// Vehicle class to transport the variable names
  class CDCPathBasicVarNames : public TrackFindingCDC::VarNames<BaseCDCPathFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcPathBasicVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcPathBasicVarNames[iName];
    }
  };

  /**
   * Var set to store basic quantities related to CDC CKF
   */
  class CDCPathBasicVarSet : public TrackFindingCDC::VarSet<CDCPathBasicVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCPathFilter::Object* path) override;

  private:
    /// use this to get the eventNumber
    StoreObjPtr<EventMetaData> m_eventMetaData;
  };
}
