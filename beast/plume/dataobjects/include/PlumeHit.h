/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PLUMEHIT_H
#define PLUMEHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassPlumeHit - digitization simulated hit for the Microtpc detector.
  *
  * This class holds particle hit data from digitization simulation. As the simulated
  * hit classes are used to generate detector response, they contain _local_
  * information.
  */
  class PlumeHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    PlumeHit(): m_sensorID(0),  m_nofPixels(0), m_posmm_u(0),  m_posmm_v(0),
      m_posmm_x(0),  m_posmm_y(0),  m_posmm_z(0) {}


    /** Standard constructor
     */
    PlumeHit(int a_sensorID, int a_nofPixels, float a_posmm_u, float a_posmm_v,
             float a_posmm_x, float a_posmm_y, float a_posmm_z)
    {
      m_sensorID = a_sensorID;
      m_nofPixels = a_nofPixels;
      m_posmm_u = a_posmm_u;
      m_posmm_v = a_posmm_v;
      m_posmm_x = a_posmm_x;
      m_posmm_y = a_posmm_y;
      m_posmm_z = a_posmm_z;
    }

    /** sensor ID */
    int getsensorID()       const { return m_sensorID; }
    /** cluster size */
    int getnofPixels()      const { return m_nofPixels; }
    /** impact position in sensor u frame, in mm */
    float getpos_u()        const { return m_posmm_u; }
    /** impact position in sensor v frame, in mm */
    float getpos_v()        const { return m_posmm_v; }
    /** impact position in G4 x frame, in mm */
    float getpos_x()        const { return m_posmm_x; }
    /** impact position in G4 y frame, in mm */
    float getpos_y()        const { return m_posmm_y; }
    /** impact position in G4 z frame, in mm */
    float getpos_z()        const { return m_posmm_z; }

  private:
    /** sensor ID */
    int m_sensorID;
    /** cluster size */
    int m_nofPixels;
    /** impact position in sensor u frame, in mm */
    float m_posmm_u;
    /** impact position in sensor v frame, in mm */
    float m_posmm_v;
    /** impact position in G4 x frame, in mm */
    float m_posmm_x;
    /** impact position in G4 y frame, in mm */
    float m_posmm_y;
    /** impact position in G4 z frame, in mm */
    float m_posmm_z;

    ClassDef(PlumeHit, 1)
  };

} // end namespace Belle2

#endif
