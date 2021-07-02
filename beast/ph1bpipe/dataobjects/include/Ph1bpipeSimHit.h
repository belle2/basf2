/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef Ph1bpipeSIMHIT_H
#define Ph1bpipeSIMHIT_H

// ROOT
#include <TObject.h>

namespace Belle2 {

  /**
    * ClassPh1bpipeSimHit - Geant4 simulated hit for the Ph1bpipe detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class Ph1bpipeSimHit : public TObject {
  public:
    /** default constructor for ROOT */
    Ph1bpipeSimHit(): m_energyDep(0), m_detNb(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     * @param detNb detector number
     */
    Ph1bpipeSimHit(float energyDep, int detNb):
      m_energyDep(energyDep), m_detNb(detNb) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the TPC number */
    int getdetNb()  const { return m_detNb; }


  private:
    /** Deposited energy in electrons */
    float m_energyDep;
    /** pipe number */
    int m_detNb;

    ClassDef(Ph1bpipeSimHit, 1)
  };

} // end namespace Belle2

#endif
