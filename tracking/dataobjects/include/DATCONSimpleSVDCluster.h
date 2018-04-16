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
  * This class stores all information about reconstructed SVD clusters.
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


    /** Get the sensor ID.
    * @return ID of the sensor.
    */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get raw sensor ID */
    unsigned short getRawSensorID() const { return m_sensorID; }

    /** Get the direction of strips.
    * @return true if u-strips, otherwise false.
    */
    bool isUCluster() const { return m_isU; }

    /** Get the coordinate of reconstructed hit.
    * The returned value is now dependent of vCluster and valid only and
    * it is only relevant for wedged/slanted sensors because of their trapezoidal shape, for rectangular shapes, the value does not change
    * @return coordinate of the reconstructed hit.
    */
    float getPosition(double v = 0) const
    {
      if (v == 0) return m_position;
      else {
        const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_sensorID);
        return (aSensorInfo->getWidth(v) / aSensorInfo->getWidth()) * m_position;
      }
    }

    /** Get collected charge.
    * @return charge collected in the cluster in e-.
    */
    unsigned short getCharge() const { return m_clsCharge; }

    /** Get seed charge.
    * @return charge of the seed strip in ADU.
    */
    unsigned short getSeedCharge() const { return m_seedCharge; }

    unsigned short getSeedStripIndex() const { return m_seedStripIndex; }

    /** Get cluster size.
    * @return number of strips contributing to the cluster.
    */
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

    //     unsigned short m_strip;    /**< Strip number of cluster, if m_clsSize is odd, middle strip of cluster, if m_clsSize is even, highest signal */

    ClassDef(DATCONSimpleSVDCluster, 2)

  }; // end class

} //Belle2 namespace

