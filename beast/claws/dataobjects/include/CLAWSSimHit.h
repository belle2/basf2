/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CLAWSSIMHIT_H
#define CLAWSSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>
#include <stdint.h>

#include <TVector3.h>
#include <algorithm>

namespace Belle2 {

  /**
    * Class CLAWSSimHit - Geant4 simulated hit for the CLAWS detector.
    * This class holds particle hit data from geant4 simulation.
    */
  class CLAWSSimHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    CLAWSSimHit(): SimHitBase(), m_time(0), m_energyDep(0), m_energyVisible(0),
      m_length(0), m_PDG(0), m_ladder(0), m_sensor(0)
    {
      std::fill_n(m_momEntry, 3, 0.0);
      std::fill_n(m_posEntry, 3, 0.0);
      std::fill_n(m_posExit, 3, 0.0);
    }

    /** Standard constructor
     * @param ladder Ladder number
     * @param sensor Sensor number
     * @param pdg PDG number of the track
     * @param time Global time
     * @param energyDep Deposited energy in GeV
     * @param energyVisble Non-ionization energy in electrons
     * @param length Track length
     * @param posEntry Entry track position
     * @param posExit Exit track position
     * @param momEntry Track momentum
     */
    CLAWSSimHit(int ladder, int sensor, int pdg, float time, float energyDep, float energyVisble,
                float length, const float* posEntry, const float* posExit, const float* momEntry):
      SimHitBase(), m_time(time), m_energyDep(energyDep), m_energyVisible(energyVisble),
      m_length(length), m_PDG(pdg), m_ladder(ladder), m_sensor(sensor)
    {
      std::copy_n(momEntry, 3, m_momEntry);
      std::copy_n(posEntry, 3, m_posEntry);
      std::copy_n(posExit, 3, m_posExit);
    }

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
    /** Return the non-ionization energy in electrons */
    float getEnergyVisible() const { return m_energyVisible; }
    /** Return the entry track position */
    TVector3 getPosEntry() const { return TVector3(m_posEntry[0], m_posEntry[1], m_posEntry[2]); }
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
    /** global time */
    float m_time;
    /** Deposited energy in GeV */
    float m_energyDep;
    /** Visible energy deposition according to Birks Law (in GeV) */
    float m_energyVisible;
    /** entry position, global coordinates */
    float m_posEntry[3];
    /** exit position, global coordinates */
    float m_posExit[3];
    /** momentum at entry, global coordinates */
    float m_momEntry[3];
    /** track length */
    float m_length;
    /** PDG number of the track */
    int m_PDG;
    /** Ladder id */
    uint8_t m_ladder;
    /** Sensor id */
    uint8_t m_sensor;

    ClassDef(CLAWSSimHit, 1)
  };

} // end namespace Belle2

#endif
