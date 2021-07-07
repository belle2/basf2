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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCAxialSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const truthAxialSegmentPairVarNames[] = {
      "truth_tanl",
      "truth_z",
      "truth_curv",
      "truth_from_alpha",
      "truth_to_alpha",
      "truth_delta_alpha",
      "truth_track_fraction",
      "__weight__",
    };

    /// Vehicle class to transport the variable names
    struct TruthAxialSegmentPairVarNames : public VarNames<CDCAxialSegmentPair> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(truthAxialSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return truthAxialSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from an axial stereo segment pair
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class TruthAxialSegmentPairVarSet : public VarSet<TruthAxialSegmentPairVarNames> {

      /// Type of the base class
      using Super = VarSet<TruthAxialSegmentPairVarNames>;

    public:
      /// Require the Monte Carlo information before the event processing starts
      void initialize() final;

      /// Signal the begin of a new event
      void beginEvent() final;

      /// Generate and assign the contained variables
      bool extract(const CDCAxialSegmentPair* ptrAxialSegmentPair) final;
    };
  }
}
