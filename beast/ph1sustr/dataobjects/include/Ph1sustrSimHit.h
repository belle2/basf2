/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef Ph1sustrSIMHIT_H
#define Ph1sustrSIMHIT_H

// ROOT
#include <TObject.h>
#include <TVector3.h>

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
