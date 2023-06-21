/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>

namespace Belle2 {

  /** The payload class for PXD cluster position error
   *
   *  The payload class stores the expected cluster position error in [cm].
   *  (Values are currently obtained from MC reco - true)
   *
   *  In principle, errors depend on cluster dimension and pixel cell size, and not on global position.
   *  (const cell size in u, 4 different sizes in v for Layer1/2 and for small/large cell regions)
   *  Nonetheless, error values are stored for each sensor in bins of u and v
   *  to accommodate the possibility to define errors as a function of position.
   *
   *  Position error is assigned to every cluster in PXDClusterizer module.
   *
   */

  class PXDClusterPositionErrorPar: public TObject {
  public:

    /** Default constructor */
    PXDClusterPositionErrorPar(unsigned short nBinsU = 1, unsigned short nBinsV = 3, unsigned short maxSize = 8,
                               float defaultValue = 0.0, float factorSensorEdge = 1.0,
                               float factorLadderJoint = 1.0, float factorDeadNeighbour = 1.0) :
      m_nBinsU(nBinsU), m_nBinsV(nBinsV), m_maxSize(maxSize), m_defaultValue(defaultValue), m_factorSensorEdge(factorSensorEdge),
      m_factorLadderJoint(factorLadderJoint), m_factorDeadNeighbour(factorDeadNeighbour), m_clusterPositionErrorMap() {}

    /** Get number of bins along sensor u side
     */
    unsigned short getBinsU() const { return m_nBinsU; }

    /** Get number of bins along sensor v side
     */
    unsigned short getBinsV() const { return m_nBinsV; }

    /** Get maximum cluster size that the error is defined
     */
    unsigned short getMaxClusterSize() const { return m_maxSize; }



    /** Get global id from uBin and vBin and cluster size
     */
    unsigned short getGlobalID(unsigned short uBin, unsigned short vBin, unsigned short size) const
    {
      if (size > m_maxSize) size = m_maxSize + 1;
      return uBin * (m_nBinsV * m_maxSize) + vBin * m_maxSize + size - 1;
    }

    /** Set map content
     *
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin) and cluser size
     * @param value cluster position error value to store
     */
    void setContent(unsigned short sensorID, unsigned short globalID, float value)
    {
      auto mapIter = m_clusterPositionErrorMap.find(sensorID);
      if (mapIter != m_clusterPositionErrorMap.end()) {
        // Already some values stored
        auto& errVec = mapIter->second;
        // Set the value
        errVec[globalID] = value;
      } else {
        // Create a fresh position error vector
        std::vector<float> errVec(m_nBinsU * m_nBinsV * m_maxSize, m_defaultValue);
        // Set the value
        errVec[globalID] = value;
        // Add vector to map
        m_clusterPositionErrorMap[sensorID] = errVec;
      }
    }

    /** Set map content
     *
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param value cluster position error value to store
     */
    void setContent(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size, float value)
    {
      setContent(sensorID, getGlobalID(uBin, vBin, size), value);
    }

    /** Get content
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin)
     * @return value cluster position error value to store
     */
    float getContent(unsigned short sensorID, unsigned short globalID) const
    {
      auto mapIter = m_clusterPositionErrorMap.find(sensorID);
      if (mapIter != m_clusterPositionErrorMap.end()) {
        // Found sensor, return position error value
        auto& errVec = mapIter->second;
        return errVec[globalID];
      }
      // Sensor not found, keep low profile and return default position error value
      return m_defaultValue;
    }

    /** Get content
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param size cluster size
     * @return value cluster position error value to store
     */
    float getContent(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size) const
    {
      return getContent(sensorID, getGlobalID(uBin, vBin, size));
    }

    /** Set scaling factor at sensor edge */
    void setSensorEdgeFactor(float factorSensorEdge)
    {
      m_factorSensorEdge = factorSensorEdge;
    }
    /** Set scaling factor at ladder joint */
    void setLadderJointFactor(float factorLadderJoint)
    {
      m_factorLadderJoint = factorLadderJoint;
    }
    /** Set scaling factor when neighbouring dead rows/columns */
    void setDeadNeighbourFactor(float factorDeadNeighbour)
    {
      m_factorDeadNeighbour = factorDeadNeighbour;
    }

    /** Get scaling factor at sensor edge */
    float getSensorEdgeFactor() const
    {
      return m_factorSensorEdge;
    }
    /** Get scaling factor at ladder joint */
    float getLadderJointFactor() const
    {
      return m_factorLadderJoint;
    }
    /** Get scaling factor when neighbouring dead rows/columns */
    float getDeadNeighbourFactor() const
    {
      return m_factorDeadNeighbour;
    }

    /** Return unordered_map with all PXD cluster position errors */
    const std::unordered_map<unsigned short, std::vector<float> >& getClusterPositionErrorMap() const {return m_clusterPositionErrorMap;}

  private:

    /** Number of bins per sensor along u side */
    unsigned short m_nBinsU;

    /** Number of bins per sensor along v side */
    unsigned short m_nBinsV;

    /** Maximum cluster size */
    unsigned short m_maxSize;

    /** Default value for map */
    float m_defaultValue;

    /** Scaling factor at sensor edge */
    float m_factorSensorEdge;

    /** Scaling factor at ladder joint */
    float m_factorLadderJoint;

    /** Scaling factor when neighbouring dead rows/colums */
    float m_factorDeadNeighbour;

    /** Map for holding the cluster position errors for all PXD sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, std::vector<float> > m_clusterPositionErrorMap;

    ClassDef(PXDClusterPositionErrorPar, 3);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
