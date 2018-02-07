/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef VXD_SIMULATION_SENSORTRAVERSAL_H
#define VXD_SIMULATION_SENSORTRAVERSAL_H

#include <G4ThreeVector.hh>
#include <vector>

namespace Belle2 {
  /** Simple struct to keep information about steps in the sensitive
   * detector.  We use this to keep the step position and energy information
   * to produce consolidated SimHits
   */
  struct StepInformation {
    /** Construct a new instance
     * @param pos step position
     * @param mom step momentum
     * @param electrons number of created electrons
     * @param time global time the step occured
     */
    StepInformation(const G4ThreeVector& stepPosition, const G4ThreeVector& stepMomentum, double stepElectrons, double stepTime,
                    double stepLength):
      position(stepPosition), momentum(stepMomentum), electrons(stepElectrons), time(stepTime), length(stepLength) {}
    /** Step position */
    G4ThreeVector position;
    /** Step momentum */
    G4ThreeVector momentum;
    /** Number of deposited electrons */
    double electrons;
    /** timestamp of the step */
    double time {0};
    /** length of the track */
    double length {0};
  };

  /** Class to keep track of the traversal of the sensitive volume for one track.
   * This class keeps the necessary information to create consolidated SimHit
   * and TrueHit objects after a particle finishes its traversal of the
   * sensitive detector
   */
  class SensorTraversal: public std::vector<StepInformation> {
  public:
    /** Iterator pair for a set of points */
    typedef std::pair<iterator, iterator> range;

    /** Use default constructor */
    SensorTraversal() = default;

    /** add a new step
     * @param position position of the step
     * @param momentum momentum of the step
     * @param electrons number of deposited electrons during step
     */
    void add(const G4ThreeVector& position, const G4ThreeVector& momentum, double electrons, double time, double length)
    {
      m_electrons += electrons;
      m_length += length;
      emplace_back(position, momentum, m_electrons, time, m_length);
    }

    /** get Geant4 trackID */
    int getTrackID() const { return m_trackID; }
    /** get PDG code of the particle */
    int getPDGCode() const { return m_pdgCode; }
    /** get total number of deposited electrons so far */
    double getElectrons() const { return m_electrons; }
    /** get flight length so far */
    double getLength() const { return m_length; }
    /** return whether the track was contained in the volume so far */
    bool isContained() const { return m_contained; }
    /** return whether the track belongs to a primary particle */
    bool isPrimary() const { return m_primary; }

    /** indicate that the track originated outisde the current volume */
    void hasEntered() { m_contained = false; }
    /** indicate that the track left the current volume */
    void hasLeft() { m_contained = false; }

    /** set initial values for a new track */
    void setInitial(int trackID, int pdgCode, bool primary)
    {
      m_trackID = trackID;
      m_pdgCode = pdgCode;
      m_primary = primary;
    }

    /** reset to be used again */
    void reset()
    {
      m_trackID = 0;
      m_pdgCode = 0;
      m_electrons = 0;
      m_length = 0;
      m_contained = true;
      m_primary = false;
      clear();
    }

  private:
    /** Geant4 Track ID */
    int m_trackID {0};
    /** PDG code for the particle */
    int m_pdgCode {0};
    /** Total number of electrons deposited by this track */
    double m_electrons {0};
    /** length of the sensor traversal */
    double m_length {0};
    /** Indication wether the track is completely contained inside the volume */
    bool m_contained {true};
    /** Indication whether track is from a primary particle */
    bool m_primary {false};
  };

} //Belle2 namespace
#endif // VXD_SIMULATION_SENSORTRAVERSAL_H
