/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <sstream>

namespace Belle2 {

  /** The DATCONSVD Cluster class
   *  This class stores the information of reconstructed SVD clusters for DATCON.
   *  It's a simplified version of the SVDCluster class
   */
  class DATCONSimpleSVDCluster: public RelationsObject {
  public:

    /** Constructor.
    * @param sensorID Sensor compact ID.
    * @param isU true if u strips, otherwise false.
    * @param position Hit position estimate.
    * @param clsCharge The cluster charge in ADC units.
    * @param seedCharge The charge of the seed strip in ADC units.
    * @param clsSize Cluster size, number of fired strips that form the cluster.
    */
    DATCONSimpleSVDCluster(VxdID sensorID, bool isU, float position,
                           unsigned short clsCharge, unsigned short seedCharge,
                           unsigned short seedStripIndex, unsigned short clsSize):
      m_sensorID(sensorID), m_isU(isU),
      m_position(position), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge), m_seedStripIndex(seedStripIndex),
      m_clsSize(clsSize)
    {}

    /** Default constructor for the ROOT IO. */
    DATCONSimpleSVDCluster(): DATCONSimpleSVDCluster(0, true, 0.0, 0, 0, 0, 0)
    {}


    /** Getter for the sensor ID. */
    VxdID getSensorID() const { return m_sensorID; }

    /** Getter for the raw sensor ID */
    unsigned short getRawSensorID() const { return m_sensorID; }

    /** Get the direction of strips.
    * @return true if u-strips, otherwise false.
    */
    bool isUCluster() const { return m_isU; }

    /** Getter for the local coordinate of the reconstructed hit.
     *  In case the cluster is a uCluster on a slanted sensor,
     *  the position returned is dependent on the position of the vCluster on the slanted sensors.
     */
    float getPosition(double v = 0) const
    {
      if (v == 0) return m_position;
      else {
        const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_sensorID);
        return (aSensorInfo->getWidth(v) / aSensorInfo->getWidth()) * m_position;
      }
    }

    /** Getter for the charge of the cluster in ADUs. */
    unsigned short getCharge() const { return m_clsCharge; }

    /** Getter for the seed charge, i.e. the charge of the seed strip, in ADUs. */
    unsigned short getSeedCharge() const { return m_seedCharge; }

    /** Getter for the index of the seed strip inside the cluster, i.e. 0...m_clsSize/2 */
    unsigned short getSeedStripIndex() const { return m_seedStripIndex; }

    /** Getter for the cluster size. */
    unsigned short getSize() const { return m_clsSize; }

    /** Get a string representation of the cluster. */
    std::string print() const
    {
      VxdID thisSensorID(m_sensorID);
      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID)
         << " side: " << ((m_isU) ? "U" : "V") << " position: " << m_position
         << " charge: " << m_clsCharge << " seed charge: "
         << m_seedCharge << " size: " << m_clsSize //<< " strip: " << m_strip
         << std::endl;
      return os.str();
    }

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if clusters of u-strips, otherwise false. */
    float m_position;          /**< Reconstructed hit position in r-phi or z. */
    unsigned short m_clsCharge;         /**< Deposited charge in ADUs. */
    unsigned short m_seedCharge;        /**< Cluster seed charge in ADUs. */
    unsigned short m_seedStripIndex;  /**< Seed strip index of the cluster (0...m_clsSize) */
    unsigned short m_clsSize;  /**< Cluster size in pixels */

    ClassDef(DATCONSimpleSVDCluster, 2)

  }; // end class

} //Belle2 namespace

