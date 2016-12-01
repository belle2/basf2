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
    static char const* const basicTrackVarNames[] = {
      "size",
      "pt",
      "sz_slope",
      //"fit_prob_3d",
      //"fit_prob_2d",
      //"fit_prob_sz",
      "drift_length_mean",
      "drift_length_variance",
      "drift_length_max",
      "drift_length_min",
      "drift_length_sum",

      "adc_mean",
      "adc_variance",
      "adc_max",
      "adc_min",
      "adc_sum",

      "empty_s_mean",
      "empty_s_variance",
      "empty_s_max",
      "empty_s_min",
      "empty_s_sum",

      "has_matching_segment",

      "s_range",
    };

    /// Vehicle class to transport the variable names
    struct BasicTrackVarSetNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      static const size_t nVars = size(basicTrackVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicTrackVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicTrackVarSet : public VarSet<BasicTrackVarSetNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCTrack* track) override;
    };
  }
}
