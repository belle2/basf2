/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <Math/Vector3D.h>
#include <Math/Point3D.h>


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
      ROOT::Math::XYZPoint emissionPoint,
      ROOT::Math::XYZVector emissionMom,
      double emissionTime,
      ROOT::Math::XYZPoint detectionPoint,
      ROOT::Math::XYZVector detectionMom,
      double detectionTime,
      double length,
      double energy_eV
    )
    {
      m_moduleID = moduleID;
      m_xe = (float) emissionPoint.X();
      m_ye = (float) emissionPoint.Y();
      m_ze = (float) emissionPoint.Z();
      m_pxe = (float) emissionMom.X();
      m_pye = (float) emissionMom.Y();
      m_pze = (float) emissionMom.Z();
      m_te = (float) emissionTime;
      m_xd = (float) detectionPoint.X();
      m_yd = (float) detectionPoint.Y();
      m_zd = (float) detectionPoint.Z();
      m_pxd = (float) detectionMom.X();
      m_pyd = (float) detectionMom.Y();
      m_pzd = (float) detectionMom.Z();
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
    ROOT::Math::XYZPoint getEmissionPoint() const {return ROOT::Math::XYZPoint(m_xe, m_ye, m_ze);}

    /**
     * Returns emission momentum direction (unit vector)
     * @return emission direction vector
     */
    ROOT::Math::XYZVector getEmissionDir() const {return ROOT::Math::XYZVector(m_pxe, m_pye, m_pze).Unit();}

    /**
     * Returns emission time
     * @return emission time
     */
    double getEmissionTime() const { return m_te; }

    /**
     * Returns detection point
     * @return detection point
     */
    ROOT::Math::XYZPoint getDetectionPoint() const {return ROOT::Math::XYZPoint(m_xd, m_yd, m_zd);}

    /**
     * Returns detection momentum direction (unit vector)
     * @return detection direction
     */
    ROOT::Math::XYZVector getDetectionDir() const {return ROOT::Math::XYZVector(m_pxd, m_pyd, m_pzd).Unit();}

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


