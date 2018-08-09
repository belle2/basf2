/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SrsensorSIMHIT_H
#define SrsensorSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {

  /**
    * ClassSrsensorSimHit - Geant4 simulated hit for the Srsensor detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class SrsensorSimHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    SrsensorSimHit(): m_energyDep(0), m_energyNiel(0), m_tkPDG(0), m_tkKEnergy(0), m_detNb(0), m_GlTime(0), m_tkPos(), m_tkMom(),
      m_tkMomDir() {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    SrsensorSimHit(float energyDep, float energyNiel, int tkPDG, float tkKEnergy, float detNb, float GlTime, const float* tkPos,
                   const float* tkMom, const float* tkMomDir):
      SimHitBase(), m_energyDep(energyDep), m_energyNiel(energyNiel), m_tkPDG(tkPDG), m_tkKEnergy(tkKEnergy), m_detNb(detNb),
      m_GlTime(GlTime)
    {
      std::copy(tkPos, tkPos + 3, m_tkPos);
      std::copy(tkMom, tkMom + 3, m_tkMom);
      std::copy(tkMomDir, tkMomDir + 3, m_tkMomDir);
    }

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the non-ionization energy in electrons */
    float getEnergyNiel()     const { return m_energyNiel; }
    /** Return the PDG number of the track */
    int gettkPDG()           const { return m_tkPDG; }
    /** Return the kinetic energy of the track */
    float gettkKEnergy()  const { return m_tkKEnergy; }
    /** Return the TPC number */
    float getdetNb()  const { return m_detNb; }
    /** Return the global time */
    float getGlTime()  const { return m_GlTime; }
    /** Return the track position */
    TVector3 gettkPos()       const { return TVector3(m_tkPos[0], m_tkPos[1], m_tkPos[2]); }
    /** Return the track momentum */
    TVector3 gettkMom()       const { return TVector3(m_tkMom[0], m_tkMom[1], m_tkMom[2]); }
    /** Return the track momentum direction */
    TVector3 gettkMomDir()    const { return TVector3(m_tkMomDir[0], m_tkMomDir[1], m_tkMomDir[2]); }
    /** Return track position as float array */
    const float* gettkPosArray() const { return m_tkPos; }
    /** Return track momentum as float array */
    const float* getrkDirArray() const { return m_tkMom; }
    /** Return track momentum direction as float array */
    const float* getrkMomDirArray() const { return m_tkMomDir; }


  private:
    /** Deposited energy in electrons */
    float m_energyDep;
    /** Non-ionization energy in electrons */
    float m_energyNiel;
    /** PDG number of the track */
    int m_tkPDG;
    /** kinetic energy of the track */
    float m_tkKEnergy;
    /** detector number */
    float m_detNb;
    /** global time */
    float m_GlTime;
    /** track position */
    float m_tkPos[3];
    /** track momentum */
    float m_tkMom[3];
    /** track momentum direction */
    float m_tkMomDir[3];


    ClassDef(SrsensorSimHit, 1)
  };

} // end namespace Belle2

#endif
