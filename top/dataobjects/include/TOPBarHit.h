/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPBARHIT_H
#define TOPBARHIT_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  /*! Class to store track parameters of incoming MC particles
   * relation to MCParticle
   * filled in top/simulation/src/SensitiveBar.cc
   */

  class TOPBarHit : public RelationsObject {
  public:

    /*! Default constructor
     */
    TOPBarHit():
      m_barID(0),
      m_pdg(0),
      m_x0(0.0),
      m_y0(0.0),
      m_z0(0.0),
      m_x(0.0),
      m_y(0.0),
      m_z(0.0),
      m_px(0.0),
      m_py(0.0),
      m_pz(0.0),
      m_time(0.0),
      m_length(0.) {
    }

    /*!  Full constructor
     * @param barID      bar ID
     * @param PDG        PDG code of particle
     * @param origin     production point
     * @param impact     impact point
     * @param momentum   impact momentum
     * @param time       impact time
     * @param length     length of particle trajectory to impact point
     */
    TOPBarHit(
      int barID,
      int PDG,
      TVector3 origin,
      TVector3 impact,
      TVector3 momentum,
      double time,
      double length
    ) {
      m_barID = barID;
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

    /*! Get bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID;}

    /*! Get PDG code of particle
     * @return PDG code
     */
    int getPDG() const { return m_pdg; }

    /*! Get production point
     * @return production position
     */
    TVector3 getProductionPoint() const { TVector3 vec(m_x0, m_y0, m_z0); return vec; }

    /*! Get impact point
     * @return impact position
     */
    TVector3 getPosition() const { TVector3 vec(m_x, m_y, m_z); return vec; }

    /*! Get impact momentum
     * @return impact momentum vector
     */
    TVector3 getMomentum() const { TVector3 vec(m_px, m_py, m_pz); return vec; }

    /*! Get tract length from production to impact point
     * @return track length
     */
    double getLength() const { return m_length; }

    /*! Get time of impact
     * @return time
     */
    double getTime() const { return m_time; }


  private:

    int m_barID;          /**< bar ID */
    int m_pdg;            /**< PDG code of particle */
    float m_x0;           /**< production point, x component */
    float m_y0;           /**< production point, y component */
    float m_z0;           /**< production point, z component */
    float m_x;            /**< impact point, x component */
    float m_y;            /**< impact point, y component */
    float m_z;            /**< impact point, z component */
    float m_px;           /**< impact momentum, x component */
    float m_py;           /**< impact momentum, y component */
    float m_pz;           /**< impact momentum, z component */
    float m_time;         /**< impact time */
    float m_length;       /**< length of particle trajectory to impact point */

    ClassDef(TOPBarHit, 1);     /**< ClassDef */

  };


} // end namespace Belle2

#endif

