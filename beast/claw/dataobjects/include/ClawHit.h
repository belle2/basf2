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
    ClawHit(): m_detNb(0), m_time(0), m_edep(0), m_mip(0), m_pe(0) {}


    /** Standard constructor
     * @param detNb Detector number
     * @param time time in seconds
     * @param edep Deposited energy in electrons
     * @param mip Particle mip
     * @param pe Particle pe
     */
    ClawHit(int detNb, int time, float edep, float mip, float pe):
      m_detNb(detNb), m_time(time), m_edep(edep), m_mip(mip), m_pe(pe)
    {
    }

    /** Return the PIN diode number */
    int getdetNb()  const { return m_detNb; }
    /** Return the time */
    int gettime() const { return m_time; }
    /** Return the energy deposited */
    float getedep() const { return m_edep; }
    /** Return MIP */
    float getMIP()  const { return m_mip; }
    /** Return PE */
    float getPE()  const { return m_pe; }

  private:

    /** Detector Number */
    int m_detNb;
    /** time */
    int m_time;
    /** Energy deposited */
    float m_edep;
    /** Particle mip */
    float m_mip;
    /** Particle pe */
    float m_pe;

    ClassDef(ClawHit, 1)
  };

} // end namespace Belle2

#endif
