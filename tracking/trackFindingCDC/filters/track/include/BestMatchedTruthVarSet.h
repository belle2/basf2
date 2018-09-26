/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Michael Eliachevitch                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr static char const* const bestMatchedTruthVarNames[] = {
      "weight", // if CDCTrack has minimalMatchPurity (50%)
      "truth_track_is_fake", // less than 80% purity in CDC
      "truth_track_is_matched", // not fake
      "truth_matched_hits",
      "truth_track_is_clone", // track is clone
      "truth", // CDC track is not clone
      "truth_first_nloops",
      "truth_event_id",
      "truth_MCTrackID",
    };

    /// Vehicle class to transport the variable names
    struct BestMatchedTruthVarNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      static const size_t nVars = size(bestMatchedTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return bestMatchedTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BestMatchedTruthVarSet : public VarSet<BestMatchedTruthVarNames> {

    public:
      /// Require the Monte Carlo truth information at initialisation
      void initialize() final;

      /// Prepare the Monte Carlo truth information at start of the event
      void beginEvent() final;

      /// Generate and assign the contained variables
      bool extract(const CDCTrack* ptrCDCTrack) override;

    private:
      /// Type of the base class
      using Super = VarSet<BestMatchedTruthVarNames>;

      StoreObjPtr<EventMetaData> m_eventMetaData;
    };
  }
}
