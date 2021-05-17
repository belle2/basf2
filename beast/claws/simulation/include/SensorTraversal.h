/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSORTRAVERSAL_H
#define SENSORTRAVERSAL_H

#include <G4ThreeVector.hh>

namespace Belle2 {
  namespace claws {
    /** Class to keep track of the traversal of the sensitive volume for one track.
     * This class keeps the necessary information to create consolidated SimHit
     * objects after a particle finishes its traversal of the sensitive
     * detector
     */
    class SensorTraversal {
    public:
      /** add a new step
       * @param position position of the step
       * @param depEnergy energy deposition
       * @param visibleEnergy visible energy deposition
       * @param length flight length so far
       */
      void add(const G4ThreeVector& position, double depEnergy, double visibleEnergy, double length)
      {
        m_depEnergy += depEnergy;
        m_visibleEnergy += visibleEnergy;
        m_length += length;
        m_exitPosition = position;
      }

      /** get Geant4 trackID */
      int getTrackID() const { return m_trackID; }
      /** get the ladder ID */
      int getLadderID() const { return m_ladderID; }
      /** get the sensor ID */
      int getSensorID() const { return m_sensorID; }
      /** get PDG code of the particle */
      int getPDGCode() const { return m_pdgCode; }
      /** get total energy deposition */
      double getDepEnergy() const { return m_depEnergy; }
      /** get visible energy deposition */
      double getVisibleEnergy() const { return m_visibleEnergy; }
      /** get flight length so far */
      double getLength() const { return m_length; }
      /** get entry time */
      double getEntryTime() const { return m_entryTime; }
      /** get entry momentum */
      const G4ThreeVector& getEntryMomentum() const { return m_entryMomentum; }
      /** get entry position */
      const G4ThreeVector& getEntryPosition() const { return m_entryPosition; }
      /** get exit position */
      const G4ThreeVector& getExitPosition() const { return m_exitPosition; }

      /** return whether the track was contained in the volume so far */
      bool isContained() const { return m_contained; }
      /** return whether the track belongs to a primary particle */
      bool isPrimary() const { return m_primary; }

      /** indicate that the track originated outisde the current volume */
      void hasEntered() { m_contained = false; }
      /** indicate that the track left the current volume */
      void hasLeft() { m_contained = false; }

      /** set initial values for a new track */
      void setInitial(int trackID, int ladderID, int sensorID, int pdgCode, bool primary, const G4ThreeVector& position,
                      const G4ThreeVector& momentum, double time)
      {
        m_trackID = trackID;
        m_ladderID = ladderID;
        m_sensorID = sensorID;
        m_pdgCode = pdgCode;
        m_primary = primary;
        m_entryPosition = position;
        m_entryMomentum = momentum;
        m_entryTime = time;
      }

      /** reset to be used again */
      void reset()
      {
        m_trackID = 0;
        m_ladderID = 0;
        m_sensorID = 0;
        m_pdgCode = 0;
        m_depEnergy = 0;
        m_visibleEnergy = 0;
        m_length = 0;
        m_contained = true;
        m_primary = false;
        m_entryTime = 0;
      }

      /** check if a track belongs to this object */
      bool check(int trackID, int ladderID, int sensorID)
      {
        // object not set yet so we can use it
        if (m_trackID == 0) return true;
        return (trackID == m_trackID && ladderID == m_ladderID && sensorID == m_sensorID);
      }

    private:
      /** Entry Position */
      G4ThreeVector m_entryMomentum;
      /** Entry Position */
      G4ThreeVector m_entryPosition;
      /** Exit Position */
      G4ThreeVector m_exitPosition;
      /** Geant4 Track ID */
      int m_trackID {0};
      /** Ladder ID */
      int m_ladderID {0};
      /** Sensor ID */
      int m_sensorID {0};
      /** PDG code for the particle */
      int m_pdgCode {0};
      /** Total deposited energy by this track */
      double m_depEnergy {0};
      /** Total visible energy by this track */
      double m_visibleEnergy {0};
      /** length of the sensor traversal */
      double m_length {0};
      /** time the track entered the volume */
      double m_entryTime {0};
      /** Indication wether the track is completely contained inside the volume */
      bool m_contained {true};
      /** Indication whether track is from a primary particle */
      bool m_primary {false};
    };

  }
} //Belle2 namespace
#endif // SENSORTRAVERSAL_H
