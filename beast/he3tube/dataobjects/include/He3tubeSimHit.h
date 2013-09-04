/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef He3tubeSIMHIT_H
#define He3tubeSIMHIT_H

// ROOT
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
    * ClassHe3tubeSimHit - Geant4 simulated hit for the He3tube detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class He3tubeSimHit : public TObject {
  public:
    /** default constructor for ROOT */
    He3tubeSimHit(): m_energyDep(0), m_detNb(0), m_tkPDG(0), m_time(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    He3tubeSimHit(float energyDep, TVector3 tkPos, int detNb, int tkPDG, float time):
      m_energyDep(energyDep), m_tkPos(tkPos), m_detNb(detNb), m_tkPDG(tkPDG), m_time(time) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the track position */
    TVector3 gettkPos()       const { return m_tkPos; }
    /** Return the detector number */
    int getdetNb()  const { return m_detNb; }
    /** Return PDG number */
    int gettkPDG()  const { return m_tkPDG; }
    /** Return global time */
    float gettime()  const { return m_time; }

  private:
    /** Deposited energy in electrons */
    float m_energyDep;
    /** track position */
    TVector3 m_tkPos;
    /** detector number */
    int m_detNb;
    /** PDG number */
    int m_tkPDG;
    /** global time */
    float m_time;

    ClassDef(He3tubeSimHit, 1)
  };

} // end namespace Belle2

#endif
