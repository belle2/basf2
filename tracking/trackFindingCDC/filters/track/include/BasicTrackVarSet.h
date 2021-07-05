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

// BOOST accumulators
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace bacc = boost::accumulators;

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr
    static char const* const basicTrackVarNames[] = {
      "pt",
      "size",
      "hits_per_layer",

      "sz_slope",
      "z0",
      "s_range",
      "avg_hit_dist",
      "has_matching_segment",

      "cont_layer_mean",
      "cont_layer_variance",
      "cont_layer_max",
      "cont_layer_min",
      "cont_layer_first",
      "cont_layer_last",
      "cont_layer_max_vs_last",
      "cont_layer_first_vs_min",
      "cont_layer_count",
      "cont_layer_occupancy",

      "super_layer_mean",
      "super_layer_variance",
      "super_layer_max",
      "super_layer_min",
      "super_layer_first",
      "super_layer_last",
      "super_layer_max_vs_last",
      "super_layer_first_vs_min",
      "super_layer_count",
      "super_layer_occupancy",

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

      "tot_mean",
      "tot_variance",
      "tot_max",
      "tot_min",
      "tot_sum",

      "empty_s_mean",
      "empty_s_variance",
      "empty_s_max",
      "empty_s_min",
      "empty_s_sum",
    };

    /// Vehicle class to transport the variable names
    struct BasicTrackVarSetNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
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

      /** Set of statistics/features to accumulate for each variable category (e.g. drift length).
       * The statistics set is defined with the boost::accumulator framework, which handles the
       * interdependencies in a smart way. E.g. the calculation of the mean reuses the previously
       * calculated sum. New statistics can be easily added if necessary.
       */
      using statistics_set = bacc::features<bacc::tag::count,
            bacc::tag::sum,
            bacc::tag::min,
            bacc::tag::max,
            bacc::tag::mean,
            bacc::tag::lazy_variance>;

      /** A boost accumulators set that aggregates statistics for the values it is called with.
          It is used to calculate the variables in the extract method.  An accumulator set should be
          created for each category of variables, such as drift length, ADC count, etc. Calling it
          can be thought of as "filling" it with new values. It then calculates all statistics
          (corresponding to the final variables) while handling interdependencies in a smart way.
      */
      using statistics_accumulator = bacc::accumulator_set<double, statistics_set>;
    };
  }
}
