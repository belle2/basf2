/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PINDIODEHIT_H
#define PINDIODEHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassPindiodeHit - digitization simulated hit for the Pindiode detector.
  *
  * This class holds particle hit data from digitization simulation. As the simulated
  * hit classes are used to generate detector response, they contain _local_
  * information.
  */
  class PindiodeHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    PindiodeHit(): m_edep(0), m_V(0), m_time(0), m_detNb(0), m_pdg(0) {}


    /** Standard constructor
     * @param edep the energy deposited
     * @param V the amplitude
     * @param time the time
     * @param detNb the PIN diode number
     * @param pdg the PDG of particles
     */
    PindiodeHit(float edep, float V, float time, int detNb, int pdg):
      m_edep(edep), m_V(V), m_time(time), m_detNb(detNb), m_pdg(pdg)
    {
    }

    /** Return the energy deposited */
    float getedep() const { return m_edep; }
    /** Return the amplitude */
    float getV() const { return m_V; }
    /** Return the time */
    float gettime() const { return m_time; }
    /** Return the PIN diode number */
    int getdetNb()  const { return m_detNb; }
    /** Return the PDG of particles */
    int getPDG()  const { return m_pdg; }

  private:

    /** Energy deposited */
    float m_edep;
    /** V */
    float m_V;
    /** time */
    float m_time;
    /** Detector Number */
    int m_detNb;
    /** Particle PDG */
    int m_pdg;

    ClassDef(PindiodeHit, 1)
  };

} // end namespace Belle2

#endif
