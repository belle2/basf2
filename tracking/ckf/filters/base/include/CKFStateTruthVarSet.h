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
#include <tracking/ckf/utilities/CKFMCUtils.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfStateTruthVarNames[] = {
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
  template<class ASeed, class AHitObject>
  class CKFStateTruthVarNames : public
    TrackFindingCDC::VarNames<CKFState<ASeed, AHitObject>> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  template<class ASeed, class AHitObject>
  class CKFStateTruthVarSet : public TrackFindingCDC::VarSet<CKFStateTruthVarNames<ASeed, AHitObject>> {
    /// The parent class
    using Super = TrackFindingCDC::VarSet<CKFStateTruthVarNames<ASeed, AHitObject>>;

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const CKFState<ASeed, AHitObject>* result) override
    {
      RecoTrack* seedTrack = result->getSeedRecoTrack();

      if (not seedTrack) return false;

      StoreObjPtr<EventMetaData> eventMetaData;
      Super::template var<Super::named("truth_event_id")>() = eventMetaData->getEvent();
      Super::template var<Super::named("truth_seed_number")>() = seedTrack->getArrayIndex();

      // Default to 0 or false (depending on context)
      Super::template var<Super::named("truth_position_x")>() = 0;
      Super::template var<Super::named("truth_position_y")>() = 0;
      Super::template var<Super::named("truth_position_z")>() = 0;
      Super::template var<Super::named("truth_momentum_x")>() = 0;
      Super::template var<Super::named("truth_momentum_y")>() = 0;
      Super::template var<Super::named("truth_momentum_z")>() = 0;
      Super::template var<Super::named("truth")>() = false;
      Super::template var<Super::named("truth_inverted")>() = true;

      if (not allStatesCorrect(*result)) {
        // Keep all variables set to false and return.
        return true;
      }

      const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

      TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
      const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*seedTrack);

      Super::template var<Super::named("truth")>() = true;
      Super::template var<Super::named("truth_inverted")>() = false;

      Super::template var<Super::named("truth_position_x")>() = cdcMCTrack->getPositionSeed().X();
      Super::template var<Super::named("truth_position_y")>() = cdcMCTrack->getPositionSeed().Y();
      Super::template var<Super::named("truth_position_z")>() = cdcMCTrack->getPositionSeed().Z();
      Super::template var<Super::named("truth_momentum_x")>() = cdcMCTrack->getMomentumSeed().X();
      Super::template var<Super::named("truth_momentum_y")>() = cdcMCTrack->getMomentumSeed().Y();
      Super::template var<Super::named("truth_momentum_z")>() = cdcMCTrack->getMomentumSeed().Z();

      return true;
    }
  };
}
