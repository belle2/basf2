/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTER_H
#define PXDCLUSTER_H

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /** The PXD Cluster class
    * This class stores all information about reconstructed PXD clusters
    * The position error matrix has the form
    * ((m_uSigma^2, m_uvRho * m_uSigma * m_vSigma),
    *  (m_uvRho * m_uSigma * m_vSigma, m_vSigma^2))
    *  The correlation coefficient is derived from the shape of the cluster.
    */
  class PXDCluster: public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDCluster():
      m_sensorID(0), m_uPosition(0), m_vPosition(0), m_uPositionSigma(1),
      m_vPositionSigma(1), m_uvRho(0),  m_clsCharge(0), m_seedCharge(0),
      m_clsSize(0), m_uSize(0), m_vSize(0), m_uStart(0), m_vStart(0) {}

    /** Deprecated constructor for backward compatibility - will be removed soon.
     * Doesn't contain error data.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     */
    PXDCluster(VxdID sensorID, float uPosition, float vPosition, float clsCharge,
               float seedCharge, unsigned short clsSize, unsigned short uSize,
               unsigned short vSize, unsigned short uStart, unsigned short vStart):
      m_sensorID(sensorID), m_uPosition(uPosition), m_vPosition(vPosition),
      m_uPositionSigma(1), m_vPositionSigma(1), m_uvRho(0),
      m_clsCharge(clsCharge), m_seedCharge(seedCharge),  m_clsSize(clsSize),
      m_uSize(uSize), m_vSize(vSize), m_uStart(uStart), m_vStart(vStart)
    {}

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param uError Error (estimate) of uPosition.
     * @param vError Error (estiamte) of vPosition.
     * @param uvRho u-v error correlation coefficient.
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     */
    PXDCluster(VxdID sensorID, float uPosition, float vPosition, float uError,
               float vError, float uvRho, float clsCharge, float seedCharge,
               unsigned short clsSize, unsigned short uSize, unsigned short vSize,
               unsigned short uStart, unsigned short vStart):
      m_sensorID(sensorID), m_uPosition(uPosition), m_vPosition(vPosition),
      m_uPositionSigma(uError), m_vPositionSigma(vError),
      m_uvRho(uvRho), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge),  m_clsSize(clsSize), m_uSize(uSize),
      m_vSize(vSize), m_uStart(uStart), m_vStart(vStart)
    {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get u coordinate of hit position.
     * @return u coordinate of hit position.
     */
    float getU() const { return m_uPosition; }

    /** Get v coordinate of hit position.
     * @return v coordinate of hit position.
     */
    float getV() const { return m_vPosition; }

    /** Get error of u coordinate of hit position.
     * @return Error of u coordinate of hit position.
     */
    float getUSigma() const { return m_uPositionSigma; }

    /** Get error in v coordinate of hit position.
     * @return Error in v coordinate of hit position.
     */
    float getVSigma() const { return m_vPositionSigma; }

    /** Get hit position error covariance coefficient
     * @return Covariance coefficient of hit position errors.
     */
    float getRho() const { return m_uvRho; }

    /** Get collected charge.
     * @return charge collected in the cluster.
     */
    float getCharge() const { return m_clsCharge; }

    /** Get seed charge.
     * @return seed charge of the cluster.
     */
    float getSeedCharge() const { return m_seedCharge; }

    /** Get cluster size.
     * @return number of pixels contributing to the cluster.
     */
    unsigned short getSize() const { return m_clsSize; }

    /** Get cluster size in u direction.
     * @return number of pixel columns contributing to the cluster.
     */
    unsigned short getUSize() const { return m_uSize; }

    /** Get cluster size in v direction.
     * @return number of pixel rows contributing to the cluster.
     */
    unsigned short getVSize() const { return m_vSize; }

    /** Get cluster start cell in u direction.
     * @return first column contributing to the cluster.
     */
    unsigned short getUStart() const { return m_uStart; }

    /** Get cluster start cell in v direction.
     * @return first row contributing to the cluster.
     */
    unsigned short getVStart() const { return m_uStart; }

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    float m_uPosition;         /**< Absolute cell position in r-phi. */
    float m_vPosition;         /**< Absolute cell position in z. */
    float m_uPositionSigma;    /**< Error in u position. */
    float m_vPositionSigma;    /**< Error in v position. */
    float m_uvRho;             /**< Cluster shape correlation. */
    float m_clsCharge;         /**< Deposited charge in electrons. */
    float m_seedCharge;        /**< Cluster seed charge in electrons. */
    unsigned short m_clsSize;  /**< Cluster size in pixels */
    unsigned short m_uSize;    /**< Cluster size in pixel columns */
    unsigned short m_vSize;    /**< Cluster size in pixel rows  */
    unsigned short m_uStart;   /**< Start column of the cluster */
    unsigned short m_vStart;   /**< Start row of the cluster */

    ClassDef(PXDCluster, 3)
  };

  /** @}*/

} //Belle2 namespace
#endif
