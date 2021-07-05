/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * @param impactLoc  impact point in local (bar) frame
     * @param theta      impact polar angle in local (bar) frame
     * @param phi        impact azimuthal angle in local (bar) frame
     */
    TOPBarHit(
      int moduleID,
      int PDG,
      TVector3 origin,
      TVector3 impact,
      TVector3 momentum,
      double time,
      double length,
      TVector3 impactLoc,
      double theta,
      double phi
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
      m_xloc = (float) impactLoc.x();
      m_yloc = (float) impactLoc.y();
      m_zloc = (float) impactLoc.z();
      m_theta = (float) theta;
      m_phi = (float) phi;
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
    TVector3 getProductionPoint() const {return TVector3(m_x0, m_y0, m_z0);}

    /**
     * Returns impact point
     * @return impact position
     */
    TVector3 getPosition() const { return TVector3(m_x, m_y, m_z);}

    /**
     * Returns impact point
     * @return impact position
     */
    TVector3 getLocalPosition() const { return TVector3(m_xloc, m_yloc, m_zloc);}

    /**
     * Returns impact momentum
     * @return impact momentum vector
     */
    TVector3 getMomentum() const {return TVector3(m_px, m_py, m_pz); }

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

    /**
     * Returns polar angle in local (bar) frame
     * @return polar angle
     */
    double getTheta() const {return m_theta;}

    /**
     * Returns azimuthal angle in local (bar) frame
     * @return azimuthal angle
     */
    double getPhi() const {return m_phi;}

  private:

    int m_moduleID = 0; /**< TOP module ID */
    int m_pdg = 0;      /**< PDG code of particle */
    float m_x0 = 0;     /**< production point, x component */
    float m_y0 = 0;     /**< production point, y component */
    float m_z0 = 0;     /**< production point, z component */
    float m_x = 0;      /**< impact point, x component */
    float m_y = 0;      /**< impact point, y component */
    float m_z = 0;      /**< impact point, z component */
    float m_px = 0;     /**< impact momentum, x component */
    float m_py = 0;     /**< impact momentum, y component */
    float m_pz = 0;     /**< impact momentum, z component */
    float m_time = 0;   /**< impact time */
    float m_length = 0; /**< length of particle trajectory to impact point */
    float m_xloc = 0;   /**< impact point, x component in local (bar) frame */
    float m_yloc = 0;   /**< impact point, y component in local (bar) frame */
    float m_zloc = 0;   /**< impact point, z component in local (bar) frame */
    float m_theta = 0;  /**< impact polar angle in local (bar) frame */
    float m_phi = 0;    /**< impact azimuthal angle in local (bar) frame */

    ClassDef(TOPBarHit, 3);     /**< ClassDef */

  };


} // end namespace Belle2


