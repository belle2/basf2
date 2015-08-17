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
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/geometry/Helix.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <array>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Equivalent to \sa TrackFindingCDCTestWithTopology for disabled tests.
    class DISABLED_Long_TrackFindingCDCTestWithSimpleSimulation :
      public TrackFindingCDCTestWithTopology {

    public:
      DISABLED_Long_TrackFindingCDCTestWithSimpleSimulation() :
        m_simpleSimulation(&(CDCWireHitTopology::getInstance()))
      {;}

      /// Preparations before the test
      virtual
      void SetUp() override
      {
        m_plotter.clear();
        m_mcAxialSegment2Ds.clear();
        m_mcSegment2Ds.clear();
        m_mcTracks.clear();
        m_mcTrajectories.clear();
        m_axialWireHits.clear();
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
        for (const Helix& helix : helices) {
          trajectories.emplace_back(helix);
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

        // Propagate the tracks and write hits to the eventtopology
        m_mcTracks = m_simpleSimulation.simulate(trajectories);

        for (size_t iTrack = 0; iTrack < m_mcTracks.size(); ++iTrack) {
          B2INFO("Size mc track " << iTrack << " : " << m_mcTracks[iTrack].size());
        }

        // Prepare the monte carlo segments
        for (const CDCTrack& mcTrack : m_mcTracks) {
          std::vector<CDCRecoSegment3D> recoSegment3DsInTrack = mcTrack.splitIntoSegments();
          for (const CDCRecoSegment3D& recoSegment3D :  recoSegment3DsInTrack) {
            m_mcSegment2Ds.push_back(recoSegment3D.projectXY());
          }
        }

        // Filter the axial segments
        for (const CDCRecoSegment2D& recoSegment2D : m_mcSegment2Ds) {
          if (recoSegment2D.getStereoType() == AXIAL) {
            m_mcAxialSegment2Ds.push_back(&recoSegment2D);
          }
        }

        // Filter for axial hits
        const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
        for (const CDCWireHit& wireHit : wireHitTopology.getWireHits()) {
          if (wireHit.isAxial()) {
            m_axialWireHits.push_back(&wireHit);
          }
        }

        m_plotter.draw(CDCWireTopology::getInstance());
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

      /// Clean up after test
      virtual void TearDown() override
      {
        m_plotter.clear();
        m_mcAxialSegment2Ds.clear();
        m_mcSegment2Ds.clear();
        m_mcTracks.clear();
        m_mcTrajectories.clear();
        m_axialWireHits.clear();
      }

      /// Get the plotter instance
      EventDataPlotter& getPlotter()
      { return m_plotter; }

    protected:
      /// Some colors  to cycle for plotting
      const std::array<std::string, 6> m_colors{{ "red", "blue", "green", "yellow", "violet", "cyan" }};

      /// Simple simulation generating the hits
      CDCSimpleSimulation m_simpleSimulation;

      /// Plotter facility to generate visual representations
      EventDataPlotter m_plotter;

      /// Memory for the Monte Carlo trajectories of the current event.
      std::vector<CDCTrajectory3D> m_mcTrajectories;

      /// Memory for the Monte Carlo tracks of the current event.
      std::vector<CDCTrack> m_mcTracks;

      /// Memory for the Monte Carlo segments of the current event.
      std::vector<CDCRecoSegment2D> m_mcSegment2Ds;

      /// Memory for the axial Monte Carlo segments of the current event.
      std::vector<const CDCRecoSegment2D*> m_mcAxialSegment2Ds;

      /// Memory for the axial hits of the current event.
      std::vector<const CDCWireHit*> m_axialWireHits;
    };

  } //end namespace TrackFindingCDC
} //end namespace Belle2
