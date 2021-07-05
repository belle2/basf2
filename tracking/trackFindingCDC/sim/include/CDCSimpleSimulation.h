/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration
    class CDCTrack;
    class CDCTrajectory3D;
    class Helix;

    /**
     *  Class providing a simple simulation of the CDC mainly for quick unit test checks.
     *
     *  Most aspects of the detection are idealized
     *  * Trajectories are ideal helices
     *  * Wires are straight lines
     *  * Drift relation linear
     *  * No detection inefficiencies are enabled
     *  * T0, TOF and signal delay in the wire are not taken into account
     *    (but can eventually implemented to study them)
     *
     *  Nevertheless drift length a smeared by one gaussian distribution of fixed width to have a
     *  realistic check for the accuracy of fast fitting procedures in terms of their chi2 distributions.
     */
    class CDCSimpleSimulation  {

    private:

      /// Structure to accomdate information about the individual hits during the simluation
      struct SimpleSimHit {

        /**
         *  Constructor from limited truth information
         *  Mainly used by the manually prepared event
         */
        SimpleSimHit(const CDCWireHit& wireHit,
                     size_t iMCTrack,
                     ERightLeft rlInfo) :
          m_wireHit(wireHit),
          m_iMCTrack(iMCTrack),
          m_rlInfo(rlInfo)
        {}

        /**
         *  Constructor from complete truth information.
         *  Mainly used in the simple simulation procedure.
         */
        SimpleSimHit(const CDCWireHit& wireHit,
                     size_t iMCTrack,
                     ERightLeft rlInfo,
                     const Vector3D& pos3D,
                     double arcLength2D = NAN,
                     double trueDriftLength = NAN) :
          m_wireHit(wireHit),
          m_iMCTrack(iMCTrack),
          m_rlInfo(rlInfo),
          m_pos3D(pos3D),
          m_arcLength2D(arcLength2D),
          m_trueDriftLength(trueDriftLength)
        {}

        /// Memory for the wire hit instance that will be given to the reconstruction
        CDCWireHit m_wireHit;

        /// Memory for the true index of the track this hit is contained in
        size_t m_iMCTrack;

        /// Memory for the true right left passage information
        ERightLeft m_rlInfo;

        /// Memory for the true position on the track closest to the wire
        Vector3D m_pos3D  = m_wireHit.getRefPos3D();

        /// Memory for the true two dimensional arc length on the helix to this hit
        double m_arcLength2D = NAN;

        /// Memory for the true drift length from the true position to the wire
        double m_trueDriftLength = NAN;
      };

    public:
      /// Getter for the wire hits created in the simulation
      ConstVectorRange<CDCWireHit> getWireHits() const;

    public:
      /**
       *  Propagates the trajectories through the CDC as without energy loss until they first leave the CDC
       *
       *  @param trajectories3D   Ideal trajectories to be propagated.
       *  @return The true tracks containing the hits generated in this process
       */
      std::vector<CDCTrack> simulate(const std::vector<CDCTrajectory3D>& trajectories3D);

      /// Same as above for one trajectory.
      CDCTrack simulate(const CDCTrajectory3D& trajectory3D);

      /// Fills the wire hits with a hard coded event from the real simulation.
      std::vector<CDCTrack> loadPreparedEvent();

    private:
      /// Creates CDCWireHits and uses them to construct the true CDCTracks.
      std::vector<CDCTrack> constructMCTracks(int nMCTracks, std::vector<SimpleSimHit> simpleSimHits);

      /// Generate hits for the given helix in starting from the two dimensional arc length.
      std::vector<SimpleSimHit> createHits(const Helix& globalHelix, double arcLength2DOffset) const;

      /// Generate connected hits for wires in the same layer close to the given wire.
      std::vector<SimpleSimHit> createHitsForLayer(const CDCWire& nearWire,
                                                   const Helix& globalHelix,
                                                   double arcLength2DOffset) const;

      /// Generate a hit for the given wire.
      SimpleSimHit createHitForCell(const CDCWire& wire,
                                    const Helix& globalHelix,
                                    double arcLength2DOffset) const;

    public:
      /// Getter for a global event time offset
      double getEventTime() const
      { return m_eventTime; }

      /// Setter for a global event time offset
      void setEventTime(double eventTime)
      { m_eventTime = eventTime; }

      /// Activate the TOF time delay
      void activateTOFDelay(bool activate = true)
      { m_addTOFDelay = activate; }

      /// Activate the in wire signal delay
      void activateInWireSignalDelay(bool activate = true)
      { m_addInWireSignalDelay = activate; }

      /// Getter for the maximal number of hits that are allowed on each layer
      int getMaxNHitOnWire() const
      { return m_maxNHitOnWire; }

      /// Setter for the maximal number of hits that are allowed on each layer
      void setMaxNHitOnWire(int maxNHitOnWire)
      { m_maxNHitOnWire = maxNHitOnWire; }

    private:
      /// Space for the memory of the generated wire hits
      std::shared_ptr<const std::vector<CDCWireHit> > m_sharedWireHits;

      /// Default drift length variance
      const double s_nominalDriftLengthVariance = 0.000169;

      /// Default in wire signal propagation speed - 27.25 cm / ns.
      const double s_nominalPropSpeed = 27.25;

      /// Default electron drift speed in cdc gas - 4 * 10^-3 cm / ns.
      const double s_nominalDriftSpeed = 4e-3;

      /// Maximal number of hits allowed on each wire (0 means all).
      int m_maxNHitOnWire = 0;

      /// A global event time
      double m_eventTime = 0;

      /// Switch to activate the addition of the time of flight.
      bool m_addTOFDelay = false;

      /// Switch to activate the in wire signal delay.
      bool m_addInWireSignalDelay = false;

      // TODO: make them freely setable ?
      /// Variance by which the drift length should be smeared.
      double m_driftLengthVariance = s_nominalDriftLengthVariance;

      /// Standard deviation by which the drift length should be smeared.
      double m_driftLengthSigma = std::sqrt(m_driftLengthVariance);

      /// Electrical current propagation speed in the wires
      double m_propSpeed = s_nominalPropSpeed;

      /// Electron drift speed in the cdc gas
      double m_driftSpeed = s_nominalDriftSpeed;
    };

  }
}
