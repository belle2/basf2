/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef Ph1sustrSIMHIT_H
#define Ph1sustrSIMHIT_H

// ROOT
#include <TObject.h>

namespace Belle2 {

  /**
    * ClassPh1sustrSimHit - Geant4 simulated hit for the Ph1sustr detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class Ph1sustrSimHit : public TObject {
  public:
    /** default constructor for ROOT */
    Ph1sustrSimHit(): m_energyDep(0), m_detID(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     * @param detID detector id
     */
    Ph1sustrSimHit(float energyDep, int detID):
      m_energyDep(energyDep), m_detID(detID) {}

    /** Return the energy deposition in electrons */
    float getEnergyDep()      const { return m_energyDep; }
    /** Return the TPC number */
    int getdetID()  const { return m_detID; }


  private:
    /** Deposited energy in electrons */
    float m_energyDep;
    /** TPC number */
    int m_detID;

    ClassDef(Ph1sustrSimHit, 1)
  };

} // end namespace Belle2

#endif
