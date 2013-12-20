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
    PindiodeSimHit(): m_energyDep(0), m_detNb(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    PindiodeSimHit(float energyDep, int detNb):
      m_energyDep(energyDep), m_detNb(detNb) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the TPC number */
    int getdetNb()  const { return m_detNb; }


  private:
    /** Deposited energy in electrons */
    float m_energyDep;
    /** detector number */
    int m_detNb;

    ClassDef(PindiodeSimHit, 1)
  };

} // end namespace Belle2

#endif
