/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PindiodeSIMHIT_H
#define PindiodeSIMHIT_H

// ROOT
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
    * ClassPindiodeSimHit - Geant4 simulated hit for the Pindiode detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class PindiodeSimHit : public TObject {
  public:
    /** default constructor for ROOT */
    PindiodeSimHit(): m_energyDep(0), m_energyNiel(0), m_tkPDG(0), m_tkKEnergy(0), m_detNb(0), m_GlTime(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    PindiodeSimHit(float energyDep, float energyNiel, TVector3 tkPos, TVector3 tkMom, TVector3 tkMomDir, int tkPDG, float tkKEnergy, float detNb, float GlTime):
      m_energyDep(energyDep), m_energyNiel(energyNiel), m_tkPos(tkPos), m_tkMom(tkMom), m_tkMomDir(tkMomDir), m_tkPDG(tkPDG), m_tkKEnergy(tkKEnergy), m_detNb(detNb), m_GlTime(GlTime) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the non-ionization energy in electrons */
    float getEnergyNiel()     const { return m_energyNiel; }
    /** Return the track position */
    TVector3 gettkPos()       const { return m_tkPos; }
    /** Return the track momentum */
    TVector3 gettkMom()       const { return m_tkMom; }
    /** Return the track momentum direction */
    TVector3 gettkMomDir()    const { return m_tkMomDir; }
    /** Return the PDG number of the track */
    int gettkPDG()           const { return m_tkPDG; }
    /** Return the kinetic energy of the track */
    float gettkKEnergy()  const { return m_tkKEnergy; }
    /** Return the TPC number */
    float getdetNb()  const { return m_detNb; }
    /** Return the global time */
    float getGlTime()  const { return m_GlTime; }

  private:
    /** Deposited energy in electrons */
    float m_energyDep;
    /** Non-ionization energy in electrons */
    float m_energyNiel;
    /** track position */
    TVector3 m_tkPos;
    /** track momentum */
    TVector3 m_tkMom;
    /** track momentum direction */
    TVector3 m_tkMomDir;
    /** PDG number of the track */
    int m_tkPDG;
    /** kinetic energy of the track */
    float m_tkKEnergy;
    /** detector number */
    float m_detNb;
    /** global time */
    float m_GlTime;

    ClassDef(PindiodeSimHit, 1)
  };

} // end namespace Belle2

#endif
