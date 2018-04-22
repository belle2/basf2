/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

#include <framework/logging/Logger.h>

#include <vector>
#include <functional>
#include <algorithm>
#include <chrono>
#include <numeric>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class to capture the time a repeated execution took
    class TimeItResult {
    public:
      /// Constructor from a series of timings.
      explicit TimeItResult(std::vector<std::chrono::duration<double> >& timeSpans) :
        m_timeSpans(timeSpans)
      {}

      /// Get the time of the individual executtions
      double getSeconds(size_t iExecution) const
      {
        return m_timeSpans.at(iExecution).count();
      }

      /// Get the average execution time
      double getAverageSeconds() const
      {
        std::chrono::duration<double> sumTimeSpan =
          std::accumulate(m_timeSpans.begin(), m_timeSpans.end(), std::chrono::duration<double>());

        std::chrono::duration<double> avgTimeSpan = sumTimeSpan / m_timeSpans.size();
        return avgTimeSpan.count();
      }

      /// Get number of executions
      size_t getNExecutions() const
      { return m_timeSpans.size(); }

      /// Print a summary of the collected time to the console
      void printSummary() const
      {
        B2INFO("First execution took " << getSeconds(0) << " seconds ");
        B2INFO("On average execution took " << getAverageSeconds() << " seconds " <<
               "in " << getNExecutions() << " executions.");
      }

    private:
      /// Memory for the time spans a repeated execution took.
      std::vector<std::chrono::duration<double> > m_timeSpans;

    };

    /// Placeholder for function call that should do nothing.
    const std::function<void()> doNothing = []() {};

    /// Execute a function n time and collect timing information about the duration of the execution.
    template<class AFunction >
    TimeItResult
    timeIt(size_t nExecutions,
           bool activateCallgrind,
           const AFunction& function,
           const std::function<void()>& setUp = doNothing,
           const std::function<void()>& tearDown = doNothing)
    {
      using namespace std::chrono;

      std::vector<duration<double>> timeSpans;
      timeSpans.reserve(nExecutions);

      for (std::size_t iExecution = 0; iExecution < nExecutions; ++iExecution) {
        setUp();

        auto now = std::chrono::high_resolution_clock::now();

        if (activateCallgrind) {
#ifdef HAS_CALLGRIND
          CALLGRIND_START_INSTRUMENTATION;
#endif
        }

        function();

        if (activateCallgrind) {
#ifdef HAS_CALLGRIND
          CALLGRIND_STOP_INSTRUMENTATION;
#endif
        }

        auto later = std::chrono::high_resolution_clock::now();

        duration<double> timeSpan = duration_cast<duration<double> >(later - now);
        timeSpans.push_back(timeSpan);
        tearDown();
      }
      return TimeItResult(timeSpans);
    }
  }
}
