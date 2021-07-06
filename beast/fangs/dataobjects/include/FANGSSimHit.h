/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef FANGSSIMHIT_H
#define FANGSSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>
#include <stdint.h>

#include <TVector3.h>
#include <algorithm>

namespace Belle2 {

  /**
    * Class FANGSSimHit - Geant4 simulated hit for the FANGS detector.
    * This class holds particle hit data from geant4 simulation.
    */
  class FANGSSimHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    FANGSSimHit(): SimHitBase(), m_trkID(0), m_ladder(0), m_sensor(0),
      m_PDG(0), m_time(0), m_energyDep(0), m_length(0)
    {
      std::fill_n(m_momEntry, 3, 0.0);
      std::fill_n(m_posEntry, 3, 0.0);
      std::fill_n(m_localposEntry, 3, 0.0);
      std::fill_n(m_posExit, 3, 0.0);
    }

    /** Standard constructor
     * @param trkID track ID
     * @param ladder the Ladder number
     * @param sensor the Sensor number
     * @param pdg the PDG number of the track
     * @param time the global time
     * @param energyDep the energy deposition in electrons
     * @param length track length
     * @param posEntry the entry track position
     * @param localposEntry the local entry track position
     * @param posExit the exit track position
     * @param momEntry the track momentum
     */
    FANGSSimHit(int trkID, int ladder, int sensor, int pdg, float time, float energyDep,
                float length, const float* posEntry, const float* localposEntry,
                const float* posExit, const float* momEntry):
      SimHitBase(), m_trkID(trkID), m_ladder(ladder), m_sensor(sensor), m_PDG(pdg),
      m_time(time), m_energyDep(energyDep), m_length(length)
    {
      std::copy_n(momEntry, 3, m_momEntry);
      std::copy_n(posEntry, 3, m_posEntry);
      std::copy_n(localposEntry, 3, m_localposEntry);
      std::copy_n(posExit, 3, m_posExit);
    }

    /** Return track ID */
    int gettrkID() const { return m_trkID; }
    /** Return the Ladder number (starting at 1, increasing with phi) */
    int getLadder() const { return m_ladder; }
    /** Return the Sensor number (starting at 1, increasing with decreasing z) */
    int getSensor() const { return m_sensor; }
    /** Return the PDG number of the track */
    int getPDG() const { return m_PDG; }
    /** Return the global time */
    float getTime() const { return m_time; }
    /** Return the energy deposition in electrons */
    float getEnergyDep() const { return m_energyDep; }
    /** Return the entry track position */
    TVector3 getPosEntry() const { return TVector3(m_posEntry[0], m_posEntry[1], m_posEntry[2]); }
    /** Return the local entry track position */
    TVector3 getLocalPosEntry() const { return TVector3(m_localposEntry[0], m_localposEntry[1], m_localposEntry[2]); }
    /** Return the exit track position */
    TVector3 getPosExit() const { return TVector3(m_posExit[0], m_posExit[1], m_posExit[2]); }
    /** Return the track momentum */
    TVector3 getMomEntry() const { return TVector3(m_momEntry[0], m_momEntry[1], m_momEntry[2]); }
    /** Return track position as float array */
    const float* getPosEntryArray() const { return m_posEntry; }
    /** Return track position as float array */
    const float* getPosExitArray() const { return m_posExit; }
    /** Return track momentum as float array */
    const float* getMomEntryArray() const { return m_momEntry; }

  private:
    /** Track ID number */
    int m_trkID;
    /** Ladder id */
    uint8_t m_ladder;
    /** Sensor id */
    uint8_t m_sensor;
    /** PDG number of the track */
    int m_PDG;
    /** global time */
    float m_time;
    /** Deposited energy in GeV */
    float m_energyDep;
    /** entry position, global coordinates */
    float m_posEntry[3];
    /** entry position, local coordinates */
    float m_localposEntry[3];
    /** exit position, global coordinates */
    float m_posExit[3];
    /** momentum at entry, global coordinates */
    float m_momEntry[3];
    /** track length */
    float m_length;



    ClassDef(FANGSSimHit, 1)
  };

} // end namespace Belle2

#endif
