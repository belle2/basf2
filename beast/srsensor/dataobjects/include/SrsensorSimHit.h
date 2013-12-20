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

// ROOT
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
    * ClassSrsensorSimHit - Geant4 simulated hit for the Srsensor detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class SrsensorSimHit : public TObject {
  public:
    /** default constructor for ROOT */
    SrsensorSimHit(): m_energyDep(0), m_detNb(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    SrsensorSimHit(float energyDep, TVector3 tkPos, int detNb):
      m_energyDep(energyDep), m_tkPos(tkPos), m_detNb(detNb) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the track position */
    TVector3 gettkPos()       const { return m_tkPos; }
    /** Return the detector number */
    int getdetNb()  const { return m_detNb; }


  private:
    /** Deposited energy in srsensor */
    float m_energyDep;
    /** track position */
    TVector3 m_tkPos;
    /** detector number */
    int m_detNb;

    ClassDef(SrsensorSimHit, 1)
  };

} // end namespace Belle2

#endif
