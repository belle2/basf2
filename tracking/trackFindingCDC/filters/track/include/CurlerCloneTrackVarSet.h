/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch, Nils Braun                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

/// Copy of `BasicTrackVarSet`, with more features, and some removed
/// TODO: Reuse code of `BasicTrackVarSet` instead of copying
/// TODO: Add eventwise features, for distinguishing clones

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    /// Mostly copied from `BasicTrackVarSet`
    constexpr
    static char const* const curlerCloneTrackVarNames[] = {
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
      "empty_s_min", // is only != 0 for CA tracks
      "empty_s_sum",

      // "has_matching_segment", // is always 0 after SegmentTrackCombiner

      "s_range",
    };

    /// Vehicle class to transport the variable names
    struct CurlerCloneTrackVarSetNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      static const size_t nVars = size(curlerCloneTrackVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return curlerCloneTrackVarNames[iName];
      }
    };

    /**
     *  Copy of `BasicTrackVarSet`, adapted for track quality indicator.
     *  Class to compute floating point variables from a track
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class CurlerCloneTrackVarSet : public VarSet<CurlerCloneTrackVarSetNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCTrack* track) override;
    };
  }
}
