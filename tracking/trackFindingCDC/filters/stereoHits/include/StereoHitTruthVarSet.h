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

#include <tracking/trackFindingCDC/filters/stereoHits/BaseStereoHitFilter.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Names of the variables to be generated
    constexpr
    static char const* const stereoHitTruthVarNames[] = {
      "truth",
      "truth_may_reversed",
      "track_is_fake_truth",
    };

    /// Vehicle class to transport the variable names
    struct StereoHitTruthVarNames : public VarNames<BaseStereoHitFilter::Object> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(stereoHitTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return stereoHitTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a stereo hit to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class StereoHitTruthVarSet : public VarSet<StereoHitTruthVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const BaseStereoHitFilter::Object* testPair) override;

      void initialize() override
      {
        CDCMCManager::getInstance().requireTruthInformation();
        VarSet<StereoHitTruthVarNames>::initialize();
      }

      void beginEvent() override
      {
        CDCMCManager::getInstance().fill();
        VarSet<StereoHitTruthVarNames>::beginEvent();
      }
    };
  }
}
