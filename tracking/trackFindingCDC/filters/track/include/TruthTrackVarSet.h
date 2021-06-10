/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr
    static char const* const truthTrackTruthVarNames[] = {
      "track_is_fake_truth",
      "truth",
    };

    /// Vehicle class to transport the variable names
    struct TruthTrackVarNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(truthTrackTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return truthTrackTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class TruthTrackVarSet : public VarSet<TruthTrackVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<TruthTrackVarNames>;

    public:
      /// Require the Monte Carlo truth information at initialisation
      void initialize() final;

      /// Prepare the Monte Carlo truth information at start of the event
      void beginEvent() final;

      /// Generate and assign the contained variables
      bool extract(const CDCTrack* track) override;
    };
  }
}
