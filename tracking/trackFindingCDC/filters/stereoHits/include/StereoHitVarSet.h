/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/filters/stereoHits/BaseStereoHitFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Names of the variables to be generated
    constexpr
    static char const* const stereoHitVarNames[] = {
      "track_size",
      "pt",
      "reco_s",
      "reco_z",
      "phi_track",
      "phi_hit",
      "theta_hit",
      "drift_length",
      "adc_count",
      "xy_distance_zero_z",
      "right_hit",
      "track_back_s",
      "track_front_s",
      "track_mean_s",
      "s_distance",
      "track_radius",
      "superlayer_id",
    };

    /// Vehicle class to transport the variable names
    struct StereoHitVarNames : public VarNames<BaseStereoHitFilter::Object> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(stereoHitVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return stereoHitVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a stereo hit to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class StereoHitVarSet : public VarSet<StereoHitVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const BaseStereoHitFilter::Object* testPair) override;
    };
  }
}
