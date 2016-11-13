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

    /// Names of the variables to be generated.
    constexpr
    static char const* const cdcTrackNames[] = {
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

    /** Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class CDCTrackVarNames : public VarNames<CDCTrack> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 20;

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return cdcTrackNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class CDCTrackVarSet : public VarSet<CDCTrackVarNames> {

    public:
      /// Construct the peeler.
      explicit CDCTrackVarSet() : VarSet<CDCTrackVarNames>() { }

      /// Generate and assign the variables from the cluster
      bool extract(const CDCTrack* track) override;
    };
  }
}
