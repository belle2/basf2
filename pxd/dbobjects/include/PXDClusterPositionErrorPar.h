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
                               float defaultValue = 0.0, float defaultFactor = 1.0):
      m_nBinsU(nBinsU), m_nBinsV(nBinsV), m_maxSize(maxSize), m_defaultValue(defaultValue), m_defaultFactor(defaultFactor),
      m_factorSensorUEdgeMap(), m_factorSensorVEdgeMap(), m_factorDeadNeighbourMap(), m_clusterPositionErrorMap() { }

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
      if (size > m_maxSize) size = m_maxSize;
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

    /** Set scaling factor at sensor U edge
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin) and cluser size
     * @param factor scale factor value to store
     */
    void setSensorUEdgeFactor(unsigned short sensorID, unsigned short globalID, float factor)
    {
      auto mapIter = m_factorSensorUEdgeMap.find(sensorID);
      if (mapIter != m_factorSensorUEdgeMap.end()) {
        // Already some values stored
        auto& facVec = mapIter->second;
        // Set the value
        facVec[globalID] = factor;
      } else {
        // Create a fresh scale factor vector
        std::vector<float> facVec(m_nBinsU * m_nBinsV * m_maxSize, m_defaultFactor);
        // Set the value
        facVec[globalID] = factor;
        // Add vector to map
        m_factorSensorUEdgeMap[sensorID] = facVec;
      }
    }
    /** Set scaling factor at sensor U edge
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param factor scale factor value to store
     */
    void setSensorUEdgeFactor(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size, float factor)
    {
      setSensorUEdgeFactor(sensorID, getGlobalID(uBin, vBin, size), factor);
    }

    /** Set scaling factor at sensor V edge
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin) and cluser size
     * @param factor scale factor value to store
     */
    void setSensorVEdgeFactor(unsigned short sensorID, unsigned short globalID, float factor)
    {
      auto mapIter = m_factorSensorVEdgeMap.find(sensorID);
      if (mapIter != m_factorSensorVEdgeMap.end()) {
        // Already some values stored
        auto& facVec = mapIter->second;
        // Set the value
        facVec[globalID] = factor;
      } else {
        // Create a fresh scale factor vector
        std::vector<float> facVec(m_nBinsU * m_nBinsV * m_maxSize, m_defaultFactor);
        // Set the value
        facVec[globalID] = factor;
        // Add vector to map
        m_factorSensorVEdgeMap[sensorID] = facVec;
      }
    }
    /** Set scaling factor at sensor V edge
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param factor scale factor value to store
     */
    void setSensorVEdgeFactor(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size, float factor)
    {
      setSensorVEdgeFactor(sensorID, getGlobalID(uBin, vBin, size), factor);
    }

    /** Set scaling factor when neighbouring dead rows/columns
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin) and cluser size
     * @param factor scale factor value to store
     */
    void setDeadNeighbourFactor(unsigned short sensorID, unsigned short globalID, float factor)
    {
      auto mapIter = m_factorDeadNeighbourMap.find(sensorID);
      if (mapIter != m_factorDeadNeighbourMap.end()) {
        // Already some values stored
        auto& facVec = mapIter->second;
        // Set the value
        facVec[globalID] = factor;
      } else {
        // Create a fresh scale factor vector
        std::vector<float> facVec(m_nBinsU * m_nBinsV * m_maxSize, m_defaultFactor);
        // Set the value
        facVec[globalID] = factor;
        // Add vector to map
        m_factorDeadNeighbourMap[sensorID] = facVec;
      }
    }
    /** Set scaling factor when neighbouring dead rows/columns
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param factor scale factor value to store
     */
    void setDeadNeighbourFactor(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size, float factor)
    {
      setDeadNeighbourFactor(sensorID, getGlobalID(uBin, vBin, size), factor);
    }

    /** Get scaling factor at sensor edge in U
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin,size)
     * @return value scale factor at sensor edge in U
     */
    float getSensorUEdgeFactor(unsigned short sensorID, unsigned short globalID) const
    {
      auto mapIter = m_factorSensorUEdgeMap.find(sensorID);
      if (mapIter != m_factorSensorUEdgeMap.end()) {
        // Found sensor, return scale factor value
        auto& facVec = mapIter->second;
        return facVec[globalID];
      }
      // Sensor not found, keep low profile and return default scale factor value
      return m_defaultFactor;
    }
    /** Get scaling factor at sensor edge in U
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param size cluster size
     * @return value scale factor at sensor edge in U
     */
    float getSensorUEdgeFactor(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size) const
    {
      return getSensorUEdgeFactor(sensorID, getGlobalID(uBin, vBin, size));
    }

    /** Get scaling factor at sensor edge in V
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin,size)
     * @return value scale factor at sensor edge in V
     */
    float getSensorVEdgeFactor(unsigned short sensorID, unsigned short globalID) const
    {
      auto mapIter = m_factorSensorVEdgeMap.find(sensorID);
      if (mapIter != m_factorSensorVEdgeMap.end()) {
        // Found sensor, return scale factor value
        auto& facVec = mapIter->second;
        return facVec[globalID];
      }
      // Sensor not found, keep low profile and return default scale factor value
      return m_defaultFactor;
    }
    /** Get scaling factor at sensor edge in V
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param size cluster size
     * @return value scale factor at sensor edge in V
     */
    float getSensorVEdgeFactor(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size) const
    {
      return getSensorVEdgeFactor(sensorID, getGlobalID(uBin, vBin, size));
    }

    /** Get scaling factor when neighbouring dead rows/column
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin,size)
     * @return value scale factor when neighbouring dead rows/column
     */
    float getDeadNeighbourFactor(unsigned short sensorID, unsigned short globalID) const
    {
      auto mapIter = m_factorDeadNeighbourMap.find(sensorID);
      if (mapIter != m_factorDeadNeighbourMap.end()) {
        // Found sensor, return scale factor value
        auto& facVec = mapIter->second;
        return facVec[globalID];
      }
      // Sensor not found, keep low profile and return default scale factor value
      return m_defaultFactor;
    }
    /** Get scaling factor when neighbouring dead rows/column
     * @param sensorID unique ID of the sensor
     * @param uBin position error bin along u side of sensor
     * @param vBin position error bin along v side of sensor
     * @param size cluster size
     * @return value scale factor when  neighbouring dead rows/columns
     */
    float getDeadNeighbourFactor(unsigned short sensorID, unsigned short uBin, unsigned short vBin, unsigned short size) const
    {
      return getDeadNeighbourFactor(sensorID, getGlobalID(uBin, vBin, size));
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

    /** Default value for scale factor map */
    float m_defaultFactor;

    /** Scaling factor at sensor edge for layers 1 or 2 and FWD/BWD sensors with sensor number 1 or 2*/
    std::unordered_map<unsigned short, std::vector<float> > m_factorSensorUEdgeMap;
    std::unordered_map<unsigned short, std::vector<float> > m_factorSensorVEdgeMap;

    /** Scaling factor when neighbouring dead rows/colums for FWD/BWD sensors by sensor number 1 or 2 */
    std::unordered_map<unsigned short, std::vector<float> > m_factorDeadNeighbourMap;

    /** Map for holding the cluster position errors for all PXD sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, std::vector<float> > m_clusterPositionErrorMap;

    ClassDef(PXDClusterPositionErrorPar, 8);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
