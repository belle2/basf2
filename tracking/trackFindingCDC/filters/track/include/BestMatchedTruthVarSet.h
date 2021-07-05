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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr static char const* const bestMatchedTruthVarNames[] = {
      // Truth: PR Track is not a fake and the best matched track (not a clone)
      "truth",
      // Other truth variables are mostly to be helpful during debugging.
      "truth_track_is_fake",
      "truth_track_is_matched",
      "truth_matched_hits",
      "truth_track_is_clone",
      "truth_first_nloops",
      "truth_event_id",
      "truth_MCTrackID",
    };

    /// Vehicle class to transport the variable names
    struct BestMatchedTruthVarNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
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

      /// Pointer to the store array object with the EventMetaData
      StoreObjPtr<EventMetaData> m_eventMetaData;
    };
  }
}
