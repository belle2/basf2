/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSIMPHOTON_H
#define TOPSIMPHOTON_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  /*! Class to store Cherenkov photons at emission and at detection
   * relation to TOPSimHit
   * filled in top/simulation/src/SensitivePMT.cc
   */

  class TOPSimPhoton : public RelationsObject {
  public:

    /*! Default constructor
     */
    TOPSimPhoton():
      m_barID(0),
      m_xe(0.0),
      m_ye(0.0),
      m_ze(0.0),
      m_pxe(0.0),
      m_pye(0.0),
      m_pze(0.0),
      m_te(0.0),
      m_xd(0.0),
      m_yd(0.0),
      m_zd(0.0),
      m_pxd(0.0),
      m_pyd(0.0),
      m_pzd(0.0),
      m_td(0.0),
      m_length(0.0),
      m_energy(0.0) {
    }

    /*!  Full constructor.
     * @param barID          bar ID
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
      int barID,
      TVector3 emissionPoint,
      TVector3 emissionMom,
      double emissionTime,
      TVector3 detectionPoint,
      TVector3 detectionMom,
      double detectionTime,
      double length,
      double energy_eV
    ) {
      m_barID = barID;
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

    /*! Get bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /*! Get emission point
     * @return emission point
     */
    TVector3 getEmissionPoint() const { TVector3 vec(m_xe, m_ye, m_ze); return vec; }

    /*! Get emission momentum direction (unit vector)
     * @return emission direction vector
     */
    TVector3 getEmissionDir() const
    { TVector3 vec(m_pxe, m_pye, m_pze); return vec.Unit(); }

    /*! Get emission time
     * @return emission time
     */
    double getEmissionTime() const { return m_te; }

    /*! Get detection point
     * @return detection point
     */
    TVector3 getDetectionPoint() const { TVector3 vec(m_xd, m_yd, m_zd); return vec; }

    /*! Get detection momentum direction (unit vector)
     * @return detection direction
     */
    TVector3 getDetectionDir() const
    { TVector3 vec(m_pxd, m_pyd, m_pzd); return vec.Unit(); }

    /*! Get detection time
     * @return detection time
     */
    double getDetectionTime() const { return m_td; }

    /*! Get propagation length
     * @return propagation length
     */
    double getLength() const { return m_length; }

    /*! Get photon energy
     * @return photon energy in [eV]
     */
    double getEnergy() const { return m_energy; }

  private:
    int m_barID;      /**< bar ID */
    float m_xe;       /**< emission point, x component */
    float m_ye;       /**< emission point, y component */
    float m_ze;       /**< emission point, z component */
    float m_pxe;      /**< emission momentum (direction), x component */
    float m_pye;      /**< emission momentum (direction), y component */
    float m_pze;      /**< emission momentum (direction), z component */
    float m_te;       /**< emission time */
    float m_xd;       /**< detection point, x component */
    float m_yd;       /**< detection point, y component */
    float m_zd;       /**< detection point, z component */
    float m_pxd;      /**< detection momentum (direction), x component */
    float m_pyd;      /**< detection momentum (direction), y component */
    float m_pzd;      /**< detection momentum (direction), z component */
    float m_td;       /**< detection time */
    float m_length;   /**< propagation length */
    float m_energy;   /**< photon energy in [eV] */

    ClassDef(TOPSimPhoton, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif

