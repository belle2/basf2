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
   * Class to store Cherenkov photons at emission and at detection
   * relation to TOPSimHit
   * filled in top/simulation/src/SensitivePMT.cc
   */

  class TOPSimPhoton : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPSimPhoton()
    {}

    /**
     * Full constructor.
     * @param moduleID          module ID
     * @param emissionPoint  emission point
     * @param emissionMom    emission momentum (direction)
     * @param emissionTime   emission time
     * @param detectionPoint detection point
     * @param detectionMom   detection momentum (direction)
     * @param detectionTime  detection time
     * @param length         photon propagation length
     * @param energy_eV      photon energy in [eV]
     */
    TOPSimPhoton(
      int moduleID,
      TVector3 emissionPoint,
      TVector3 emissionMom,
      double emissionTime,
      TVector3 detectionPoint,
      TVector3 detectionMom,
      double detectionTime,
      double length,
      double energy_eV
    )
    {
      m_moduleID = moduleID;
      m_xe = (float) emissionPoint.x();
      m_ye = (float) emissionPoint.y();
      m_ze = (float) emissionPoint.z();
      m_pxe = (float) emissionMom.x();
      m_pye = (float) emissionMom.y();
      m_pze = (float) emissionMom.z();
      m_te = (float) emissionTime;
      m_xd = (float) detectionPoint.x();
      m_yd = (float) detectionPoint.y();
      m_zd = (float) detectionPoint.z();
      m_pxd = (float) detectionMom.x();
      m_pyd = (float) detectionMom.y();
      m_pzd = (float) detectionMom.z();
      m_td = (float) detectionTime;
      m_length = (float) length;
      m_energy = (float) energy_eV;
    }

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const { return m_moduleID; }

    /**
     * Returns emission point
     * @return emission point
     */
    TVector3 getEmissionPoint() const { TVector3 vec(m_xe, m_ye, m_ze); return vec; }

    /**
     * Returns emission momentum direction (unit vector)
     * @return emission direction vector
     */
    TVector3 getEmissionDir() const
    { TVector3 vec(m_pxe, m_pye, m_pze); return vec.Unit(); }

    /**
     * Returns emission time
     * @return emission time
     */
    double getEmissionTime() const { return m_te; }

    /**
     * Returns detection point
     * @return detection point
     */
    TVector3 getDetectionPoint() const { TVector3 vec(m_xd, m_yd, m_zd); return vec; }

    /**
     * Returns detection momentum direction (unit vector)
     * @return detection direction
     */
    TVector3 getDetectionDir() const
    { TVector3 vec(m_pxd, m_pyd, m_pzd); return vec.Unit(); }

    /**
     * Returns detection time
     * @return detection time
     */
    double getDetectionTime() const { return m_td; }

    /**
     * Returns propagation length
     * @return propagation length
     */
    double getLength() const { return m_length; }

    /**
     * Returns photon energy
     * @return photon energy in [eV]
     */
    double getEnergy() const { return m_energy; }

  private:
    int m_moduleID = 0;   /**< TOP module ID */
    float m_xe = 0;       /**< emission point, x component */
    float m_ye = 0;       /**< emission point, y component */
    float m_ze = 0;       /**< emission point, z component */
    float m_pxe = 0;      /**< emission momentum (direction), x component */
    float m_pye = 0;      /**< emission momentum (direction), y component */
    float m_pze = 0;      /**< emission momentum (direction), z component */
    float m_te = 0;       /**< emission time */
    float m_xd = 0;       /**< detection point, x component */
    float m_yd = 0;       /**< detection point, y component */
    float m_zd = 0;       /**< detection point, z component */
    float m_pxd = 0;      /**< detection momentum (direction), x component */
    float m_pyd = 0;      /**< detection momentum (direction), y component */
    float m_pzd = 0;      /**< detection momentum (direction), z component */
    float m_td = 0;       /**< detection time */
    float m_length = 0;   /**< propagation length */
    float m_energy = 0;   /**< photon energy in [eV] */

    ClassDef(TOPSimPhoton, 2); /**< ClassDef */

  };


} // end namespace Belle2


