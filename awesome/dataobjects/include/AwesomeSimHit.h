/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef AwesomeSIMHIT_H
#define AwesomeSIMHIT_H

// ROOT
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
    * ClassAwesomeSimHit - Geant4 simulated hit for the Awesome detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class AwesomeSimHit : public RelationsObject {
  public:
    /** default constructor for ROOT */
    AwesomeSimHit(): m_energyDep(0) {}

    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    AwesomeSimHit(float energyDep): m_energyDep(energyDep) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()        const { return m_energyDep; }
  private:
    /** Deposited energy in electrons */
    float m_energyDep;

    ClassDef(AwesomeSimHit, 1)
  };

} // end namespace Belle2

#endif
