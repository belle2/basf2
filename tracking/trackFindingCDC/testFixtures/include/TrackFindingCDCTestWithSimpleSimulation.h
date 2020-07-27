/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>
#include <tracking/trackFindingCDC/sim/CDCSimpleSimulation.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/geometry/Helix.h>

#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>
#include <tracking/trackFindingCDC/utilities/TimeIt.h>

#include <array>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Equivalent to \sa TrackFindingCDCTestWithTopology for disabled tests.
    class TrackFindingCDCTestWithSimpleSimulation :
      public TrackFindingCDCTestWithTopology {

    public:
      TrackFindingCDCTestWithSimpleSimulation() : m_simpleSimulation()
      {
      }

      /// Preparations before the test

      void SetUp() override
      {

        m_savedLogLevel = Belle2::LogSystem::Instance().getLogConfig()->getLogLevel();
        m_savedDebugLogInfo = Belle2::LogSystem::Instance().getLogConfig()->getLogInfo(LogConfig::c_Debug);
        bool run_disabled = ::testing::GTEST_FLAG(also_run_disabled_tests);
        if (run_disabled) {
          Belle2::LogSystem::Instance().getLogConfig()->setLogLevel(LogConfig::ELogLevel::c_Debug);
          Belle2::LogSystem::Instance().getLogConfig()->setLogInfo(LogConfig::c_Debug, LogConfig::c_Level + LogConfig::c_Message);
        }

        m_plotter.clear();
        m_mcAxialSegment2Ds.clear();
        m_mcSegment2Ds.clear();
        m_mcTracks.clear();
        m_mcTrajectories.clear();
        m_axialWireHits.clear();
        m_wireHits.clear();
      }

      /// Populate the event with hits generated from the given helices.
      void simulate(const std::initializer_list<Helix>& helices)
      {
        simulate(std::vector<Helix>(helices));
      }


      /// Populate the event with hits generated from the given helices.
      void simulate(const std::vector<Helix>& helices)
      {
        std::vector<CDCTrajectory3D> trajectories;
        trajectories.reserve(helices.size());
        for (const Helix& helix : helices) {
          // had to make the implicit convesion to an explicit conversion. Maybe there is a more elegant way to do it
          trajectories.emplace_back(UncertainHelix(helix));
        }
        simulate(trajectories);
      }

      /// Populate the event with hits generated from the given trajectories.
      void simulate(const std::initializer_list<CDCTrajectory3D>& trajectories)
      {
        simulate(std::vector<CDCTrajectory3D>(trajectories));
      }

      /// Populate the event with hits generated from the given trajectories.
      void simulate(const std::vector<CDCTrajectory3D>& trajectories)
      {
        m_mcTrajectories = trajectories;

        // Construct tracks. Wire hits are stored in the simple simulation
        m_mcTracks = m_simpleSimulation.simulate(trajectories);

        fillCaches();
      }

      /// Populate the event with hits hard codes presimulated hits
      void loadPreparedEvent()
      {
        // No trajectory information present
        m_mcTrajectories.clear();

        // Load prepared tracks wire hits are stored in the simple simulation
        m_mcTracks = m_simpleSimulation.loadPreparedEvent();

        fillCaches();
      }

      /// Prepare a set of hits, axial hits, segments, axial segments and track hits.
      void fillCaches()
      {
        for (size_t iTrack = 0; iTrack < m_mcTracks.size(); ++iTrack) {
          B2INFO("Size mc track " << iTrack << " : " << m_mcTracks[iTrack].size());
        }

        // Prepare the monte carlo segments
        for (const CDCTrack& mcTrack : m_mcTracks) {
          std::vector<CDCSegment3D> segment3DsInTrack = mcTrack.splitIntoSegments();
          for (const CDCSegment3D& segment3D :  segment3DsInTrack) {
            m_mcSegment2Ds.push_back(segment3D.stereoProjectToRef());
          }
        }

        // Filter the axial segments
        for (const CDCSegment2D& segment2D : m_mcSegment2Ds) {
          if (segment2D.getStereoKind() == EStereoKind::c_Axial) {
            m_mcAxialSegment2Ds.push_back(&segment2D);
          }
        }

        // Filter for axial hits
        for (const CDCWireHit& wireHit : m_simpleSimulation.getWireHits()) {
          if (wireHit.isAxial()) {
            m_axialWireHits.push_back(&wireHit);
          }
        }

        // Pick up points for all hits
        for (const CDCWireHit& wireHit : m_simpleSimulation.getWireHits()) {
          m_wireHits.push_back(&wireHit);
        }

        m_plotter.draw(CDCWireTopology::getInstance());
        for (const CDCWireHit& wireHit : m_simpleSimulation.getWireHits()) {
          m_plotter.draw(wireHit);
        }
      }

      /// Add the Monte Carlo tracks to the event plot
      void plotMCTracks()
      { for (const CDCTrack& mcTrack : m_mcTracks) m_plotter.draw(mcTrack); }

      /// Add the Monte Carlo trajectories to the event plot
      void plotMCTrajectories()
      {
        for (const CDCTrajectory3D& mcTrajectory : m_mcTrajectories) {
          m_plotter.draw(mcTrajectory.getTrajectory2D());
        }
      }

      /// Save content of the plotter to svg file only if running in unrestricted mode --gtest_also_run_disabled_tests
      void saveDisplay(const std::string& svgFileName)
      {
        bool run_disabled = ::testing::GTEST_FLAG(also_run_disabled_tests);
        if (run_disabled) {
          m_plotter.save(svgFileName);
        } else {
          B2INFO("Not writing display file. To activate svg display output run with --gtest_also_run_disabled_tests");
        }
      }

      /** Repeat a time critical simulation section a couple of times.
       *
       *  The repeated execution is only done if the test is run in unrestricted mode.
       *  The default is to run it once.
       */
      template<class AFunction >
      TimeItResult
      timeIt(size_t nExecutions,
             bool activateCallgrind,
             const AFunction& function,
             const std::function<void()>& setUp = doNothing,
             const std::function<void()>& tearDown = doNothing)
      {
        bool run_disabled = ::testing::GTEST_FLAG(also_run_disabled_tests);
        if (not run_disabled) {
          nExecutions = 1;
        }
        return Belle2::TrackFindingCDC::timeIt(nExecutions,
                                               activateCallgrind,
                                               function,
                                               setUp,
                                               tearDown);

      }

      /// Forwarding draw class to the plotter instance
      template<class... Ts>
      void draw(Ts&& ... args)
      {
        m_plotter.draw(std::forward<Ts>(args) ...);
      }

      /// Clean up after test
      void TearDown() override
      {
        Belle2::LogSystem::Instance().getLogConfig()->setLogLevel(m_savedLogLevel);
        Belle2::LogSystem::Instance().getLogConfig()->setLogInfo(LogConfig::c_Debug, m_savedDebugLogInfo);

        m_plotter.clear();
        m_mcAxialSegment2Ds.clear();
        m_mcSegment2Ds.clear();
        m_mcTracks.clear();
        m_mcTrajectories.clear();
        m_axialWireHits.clear();
        m_wireHits.clear();
      }

    protected:
      /// Memory for the log level that was set before the test.
      LogConfig::ELogLevel m_savedLogLevel = LogConfig::c_Info;

      /// Memory for the log info of debug that was set before the test.
      unsigned int m_savedDebugLogInfo = 100;

      /// Some colors  to cycle for plotting
      const std::array<std::string, 6> m_colors{{ "red", "blue", "green", "yellow", "violet", "cyan" }};

      /// Simple simulation generating the hits
      CDCSimpleSimulation m_simpleSimulation;

      /// Memory for the Monte Carlo trajectories of the current event.
      std::vector<CDCTrajectory3D> m_mcTrajectories;

      /// Memory for the Monte Carlo tracks of the current event.
      std::vector<CDCTrack> m_mcTracks;

      /// Memory for the Monte Carlo segments of the current event.
      std::vector<CDCSegment2D> m_mcSegment2Ds;

      /// Memory for the axial Monte Carlo segments of the current event.
      std::vector<const CDCSegment2D*> m_mcAxialSegment2Ds;

      /// Memory for the axial hits of the current event.
      std::vector<const CDCWireHit*> m_axialWireHits;

      /// Memory for the hits of the current event.
      std::vector<const CDCWireHit*> m_wireHits;

    private:
      /// Plotter facility to generate visual representations
      EventDataPlotter m_plotter;

    };

  }
}
