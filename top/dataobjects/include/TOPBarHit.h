/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Class to store track parameters of incoming MC particles
   * relation to MCParticle
   * filled in top/simulation/src/SensitiveBar.cc
   */

  class TOPBarHit : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPBarHit()
    {}

    /**
     * Full constructor
     * @param moduleID   module ID
     * @param PDG        PDG code of particle
     * @param origin     production point
     * @param impact     impact point
     * @param momentum   impact momentum
     * @param time       impact time
     * @param length     length of particle trajectory to impact point
     */
    TOPBarHit(
      int moduleID,
      int PDG,
      TVector3 origin,
      TVector3 impact,
      TVector3 momentum,
      double time,
      double length
    )
    {
      m_moduleID = moduleID;
      m_pdg = PDG;
      m_x0 = (float) origin.x();
      m_y0 = (float) origin.y();
      m_z0 = (float) origin.z();
      m_x = (float) impact.x();
      m_y = (float) impact.y();
      m_z = (float) impact.z();
      m_px = (float) momentum.x();
      m_py = (float) momentum.y();
      m_pz = (float) momentum.z();
      m_time = (float) time;
      m_length = (float) length;
    }

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const { return m_moduleID;}

    /**
     * Returns PDG code of particle
     * @return PDG code
     */
    int getPDG() const { return m_pdg; }

    /**
     * Returns production point
     * @return production position
     */
    TVector3 getProductionPoint() const { TVector3 vec(m_x0, m_y0, m_z0); return vec; }

    /**
     * Returns impact point
     * @return impact position
     */
    TVector3 getPosition() const { TVector3 vec(m_x, m_y, m_z); return vec; }

    /**
     * Returns impact momentum
     * @return impact momentum vector
     */
    TVector3 getMomentum() const { TVector3 vec(m_px, m_py, m_pz); return vec; }

    /**
     * Returns tract length from production to impact point
     * @return track length
     */
    double getLength() const { return m_length; }

    /**
     * Returns time of impact
     * @return time
     */
    double getTime() const { return m_time; }


  private:

    int m_moduleID = 0;       /**< TOP module ID */
    int m_pdg = 0;            /**< PDG code of particle */
    float m_x0 = 0;           /**< production point, x component */
    float m_y0 = 0;           /**< production point, y component */
    float m_z0 = 0;           /**< production point, z component */
    float m_x = 0;            /**< impact point, x component */
    float m_y = 0;            /**< impact point, y component */
    float m_z = 0;            /**< impact point, z component */
    float m_px = 0;           /**< impact momentum, x component */
    float m_py = 0;           /**< impact momentum, y component */
    float m_pz = 0;           /**< impact momentum, z component */
    float m_time = 0;         /**< impact time */
    float m_length = 0;       /**< length of particle trajectory to impact point */

    ClassDef(TOPBarHit, 2);     /**< ClassDef */

  };


} // end namespace Belle2


