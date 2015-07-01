/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLAWHIT_H
#define CLAWHIT_H

#include <simulation/dataobjects/SimHitBase.h>
//#include <framework/datastore/RelationsObject.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {
  /**
  * ClassClawHit - digitization simulated hit for the Claw detector.
  *
  * This class holds particle hit data from digitization simulation. As the simulated
  * hit classes are used to generate detector response, they contain _local_
  * information.
  */
  class ClawHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    ClawHit(): m_detNb(0), m_time(0), m_edep(0), m_counter(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    ClawHit(int detNb, int time, float edep, int counter):
      m_detNb(detNb), m_time(time), m_edep(edep), m_counter(counter)
    {
    }

    /** Return the PIN diode number */
    int getdetNb()  const { return m_detNb; }
    /** Return the time */
    int gettime() const { return m_time; }
    /** Return the energy deposited */
    float getedep() const { return m_edep; }
    /** Return the PDG of particles */
    int getcounter()  const { return m_counter; }

  private:

    /** Detector Number */
    int m_detNb;
    /** time */
    int m_time;
    /** Energy deposited */
    float m_edep;
    /** Particle PDG */
    int m_counter;

    ClassDef(ClawHit, 1)
  };

} // end namespace Belle2

#endif
