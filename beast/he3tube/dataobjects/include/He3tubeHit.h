/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef He3tubeHIT_H
#define He3tubeHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassHe3Hit - digitization simulated hit for the He3tube detector.
  *
  * This class holds particle hit data from digitization simulation. As the simulated
  * hit classes are used to generate detector response, they contain _local_
  * information.
  */
  class He3tubeHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    He3tubeHit(): m_energyDep(0), m_detNb(0), m_peakV(0), m_time(0), m_definiteNeutron(0) {}


    /** Standard constructor
     * @param energyDep the energy deposition in electrons
     * @param detNb the tube number
     * @param peakV peak
     * @param time time of hit
     * @param definiteNeutron true if this is definitely a neutron event
     */
    He3tubeHit(double energyDep, int detNb, double peakV, double time, bool definiteNeutron)
    {
      m_energyDep = energyDep;
      m_detNb = detNb;
      m_peakV = peakV;
      m_time = time;
      m_definiteNeutron = definiteNeutron;
    }

    /** Return the energy deposition in electrons */
    double getEnergyDep()      const { return m_energyDep; }
    /** Return the tube number */
    int getdetNb()  const { return m_detNb; }
    /** Return peak */
    double getPeakV() const { return m_peakV; }
    /** Return time of hit */
    double getTime() const { return m_time; }
    /** true if this is definitely a neutron event */
    bool definiteNeutron() const { return m_definiteNeutron;}

  private:
    /** Deposited energy in electrons */
    double m_energyDep;
    /** Detector Number */
    int m_detNb;
    /** waveform peak */
    double m_peakV;
    /** time of hit */
    double m_time;
    /** true if this is definitely a neutron event */
    bool m_definiteNeutron;

    ClassDef(He3tubeHit, 1)
  };

} // end namespace Belle2

#endif
