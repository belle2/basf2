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

#include <TObject.h>
#include <vxd/VxdID.h>

namespace Belle2 {

  /** The PXD Cluster class
   * This class stores all information about reconstructed PXD clusters
   */
  class PXDCluster: public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDCluster():
        m_sensorID(0), m_uPosition(0), m_vPosition(0),
        m_clsCharge(0), m_seedCharge(0),
        m_clsSize(0), m_uSize(0), m_vSize(0) {}

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     */
    PXDCluster(VxdID sensorID, float uPosition, float vPosition, float clsCharge, float seedCharge, unsigned short clsSize, unsigned short uSize, unsigned short vSize):
        m_sensorID(sensorID), m_uPosition(uPosition), m_vPosition(vPosition),
        m_clsCharge(clsCharge), m_seedCharge(seedCharge),
        m_clsSize(clsSize), m_uSize(uSize), m_vSize(vSize) {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get u coordinate of cell center.
     * @return u coordinate of cluster position.
     */
    float getU() const { return m_uPosition; }

    /** Get v coordinate of cell center.
     * @return v coordinate of cluster position.
     */
    float getV() const { return m_vPosition; }

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

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    float m_uPosition;         /**< Absolute cell position in r-phi. */
    float m_vPosition;         /**< Absolute cell position in z. */
    float m_clsCharge;         /**< Deposited charge in electrons. */
    float m_seedCharge;        /**< Cluster seed charge in electrons. */
    unsigned short m_clsSize;  /**< Cluster size in pixels */
    unsigned short m_uSize;    /**< Cluster size in pixel columns */
    unsigned short m_vSize;    /**< Cluster size in pixel rows  */

    ClassDef(PXDCluster, 1)

  };
} //Belle2 namespace
#endif
