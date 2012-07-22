/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDSimpleDigiHIT_H
#define VXDSimpleDigiHIT_H

#include <vxd/dataobjects/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>

// ROOT
#include <TObject.h>


namespace Belle2 {

  /**
    * Class VXDSimpleDigiHit - This class is the output of the VXDSimpleBackground module
    *
    * The VXDSimpleDigiHit knows if the measurement standard deviation is correct or not
    * in other words if the true measurement error used for generating the smearing is
    * the same as written is given in m_sigU and m_sigV.
    */
  class VXDSimpleDigiHit : public TObject {

  public:
    /** Default constructor for ROOT IO */
    VXDSimpleDigiHit(): m_sensorID(0), m_u(0), m_v(0), m_sigU(0), m_sigV(0), m_vxdTrueHitPtr(NULL), m_statusOfSigmas(false) {}

    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param sigU Measurement error of u coordinate
     * @param sigV Measurement error of v coordinate
     * @param vxdTrueHitPtr Pointer to the VXDTrueHit if info in this VXDSimpleDigiHit came from a VXDTrueHit
     * @param statusOfSigmas states if the sigU and sigV are the real errors or not
     */
    VXDSimpleDigiHit(VxdID sensorID, float u, float v, float sigU, float sigV, const VXDTrueHit* vxdTrueHitPtr, bool statusOfSigmas):
      m_sensorID(sensorID), m_u(u), m_v(v), m_sigU(sigU), m_sigV(sigV), m_vxdTrueHitPtr(vxdTrueHitPtr), m_statusOfSigmas(statusOfSigmas)  {}
    /** Return the Sensor ID */
    VxdID getSensorID() const { return m_sensorID; }
    /** Return local u coordinate of hit */
    float getU() const { return m_u; }
    /** Return local v coordinate of hit */
    float getV() const { return m_v; }
    /** Return sigma u of hit */
    float getSigU() const { return m_sigU; }
    /** Return sigma v of hit */
    float getSigV() const { return m_sigV; }
    /** Return pointer to TrueHit used for creation of this object. Can be NULL if this is a background hit */
    const VXDTrueHit* getTrueHit() const { return m_vxdTrueHitPtr; }
    /** Return  */
    bool isVarianceCorrect() const { return m_statusOfSigmas; }

  protected:

    VxdID m_sensorID;         /**< ID of the sensor */
    float m_u;                /**< Local u coordinate when crossing detector plane*/
    float m_v;                /**< Local v coordinate when crossing detector plane*/
    float m_sigU;             /**< Measurement error of u coordinate */
    float m_sigV;             /**< Measurement error of v coordinate */

    const VXDTrueHit* m_vxdTrueHitPtr; /**< Pointer to the VXDTrueHit that was used to create this VXDSimpleDigiHit*/
    bool m_statusOfSigmas; /**< set to true if this hit was smeared with the variance set in the recoHit*/


    ClassDef(VXDSimpleDigiHit, 2)
  };

} // end namespace Belle2

#endif
