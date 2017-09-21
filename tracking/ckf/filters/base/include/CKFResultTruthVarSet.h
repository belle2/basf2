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
#include <tracking/ckf/utilities/CKFMCUtils.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <string>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfResultTruthNames[] = {
    "truth",
    "truth_number_of_correct_hits",
    "truth_number_of_mc_pxd_hits",
    "truth_number_of_mc_svd_hits",
    "truth_number_of_mc_cdc_hits",
    "truth_event_number",
    "truth_seed_number",
    "truth_teacher"
  };

  /// Vehicle class to transport the variable names
  template<class ASeedObject, class AHitObject>
  class CKFResultTruthVarNames : public
    TrackFindingCDC::VarNames<CKFResultObject<ASeedObject, AHitObject>> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfResultTruthNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfResultTruthNames[iName];
    }
  };

  /**
   * Var set used in the CKF for calculating the probability of a correct result,
   * which knows the truth information if two tracks belong together or not.
   */
  template<class ASeedObject, class AHitObject>
  class CKFResultTruthVarSet : public TrackFindingCDC::VarSet<CKFResultTruthVarNames<ASeedObject, AHitObject>> {
    /// Parent class
    using Super = TrackFindingCDC::VarSet<CKFResultTruthVarNames<ASeedObject, AHitObject>>;

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const CKFResultObject<ASeedObject, AHitObject>* result) final {
      RecoTrack* seedTrack = result->getSeed();
      if (not seedTrack) return false;

      StoreObjPtr<EventMetaData> eventMetaData;
      Super::template var<Super::named("truth_event_number")>() = eventMetaData->getEvent();
      Super::template var<Super::named("truth_seed_number")>() = seedTrack->getArrayIndex();
      Super::template var<Super::named("truth_teacher")>() = result->getTeacherInformation();

      const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

      TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
      const RecoTrack* mcTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*seedTrack);

      // Default to false
      Super::template var<Super::named("truth_number_of_correct_hits")>() = 0;
      Super::template var<Super::named("truth")>() = 0;
      Super::template var<Super::named("truth_number_of_mc_pxd_hits")>() = 0;
      Super::template var<Super::named("truth_number_of_mc_svd_hits")>() = 0;
      Super::template var<Super::named("truth_number_of_mc_cdc_hits")>() = 0;

      if (not mcTrack)
      {
        // track is a fake.
        return true;
      }

      // Count the number of times the related MC-track is also related to the clusters.
      const unsigned int numberOfCorrectHits = getNumberOfCorrectHits(mcTrack, result->getHits());

      Super::template var<Super::named("truth_number_of_correct_hits")>() = numberOfCorrectHits;
      Super::template var<Super::named("truth_number_of_mc_svd_hits")>() = mcTrack->getNumberOfSVDHits();
      Super::template var<Super::named("truth_number_of_mc_pxd_hits")>() = mcTrack->getNumberOfPXDHits();
      Super::template var<Super::named("truth_number_of_mc_cdc_hits")>() = mcTrack->getNumberOfCDCHits();
      Super::template var<Super::named("truth")>() = numberOfCorrectHits == result->getHits().size() and numberOfCorrectHits > 0;

      return true;
    }
  };
}
