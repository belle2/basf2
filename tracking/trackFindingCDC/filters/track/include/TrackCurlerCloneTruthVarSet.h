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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr
    static char const* const trackCurlerCloneTruthVarNames[] = {
      "weight", // 0 for fakes, 1 for MC matched tracks
      "track_is_curler_clone_truth", // track is clone
      "truth", // not clone, either best match or fake (which is then not used due to weight 0)
    };

    /// Vehicle class to transport the variable names
    struct TrackCurlerCloneTruthVarNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      static const size_t nVars = size(trackCurlerCloneTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return trackCurlerCloneTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class TrackCurlerCloneTruthVarSet : public VarSet<TrackCurlerCloneTruthVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<TrackCurlerCloneTruthVarNames>;

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
