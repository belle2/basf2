/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>

namespace Belle2 {

  /** The payload class for PXD cluster charge calibrations
   *
   *  The payload class stores the median cluster charge values
   *  for PXD sensors on a grid nBinsU x nBinsV. The values are
   *  stored in raw ADC units (called ADU).
   *
   *  The granularity of the grid is user adjustable. The default
   *  value of a missing calibraiton is -1.0.
   */

  class PXDClusterLandauMapPar: public TObject {
  public:

    /** Default constructor */
    PXDClusterLandauMapPar(int nBinsU = 4, int nBinsV = 6, std::pair<float, float> defaultLandau = std::make_pair(-1,
                           1)) : m_nBinsU(nBinsU), m_nBinsV(nBinsV),
      m_landau_par(defaultLandau), m_sensorCalibrationMap() {}

    /** Get number of bins along sensor u side
     */
    unsigned short getBinsU() const { return m_nBinsU; }

    /** Get number of bins along sensor v side
     */
    unsigned short getBinsV() const { return m_nBinsV; }

    /** Get global id from uBin and vBin
     */
    unsigned short getGlobalID(unsigned short uBin, unsigned short vBin) const { return uBin * m_nBinsV + vBin; }

    /** Set map content
     *
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin)
     * @param MPV calibration value to store
     * @param sigma calibration value to store
     */
    void setContent(unsigned short sensorID, unsigned short globalID, std::pair<float, float> landau_par)
    {
      auto mapIter = m_sensorCalibrationMap.find(sensorID);
      if (mapIter != m_sensorCalibrationMap.end()) {
        // Already some values stored
        auto& calVec = mapIter->second;
        // Set the value
        calVec[globalID] = landau_par;
      } else {
        // Create a fresh calibration vector and initialise with MPV and sigma
        std::vector<std::pair<float, float>> calVec(m_nBinsU * m_nBinsV, landau_par);
        // Set the value

        //calVec[globalID] = value;
        // Add vector to map
        m_sensorCalibrationMap[sensorID] = calVec;
      }
    }

    /** Set map content
     *
     * @param sensorID unique ID of the sensor
     * @param uBin calibration bin along u side of sensor
     * @param vBin calibration bin along v side of sensor
     * @return value calibration value to store
     */
    void setContent(unsigned short sensorID, unsigned short uBin, unsigned short vBin, std::pair<float, float> landau_par)
    {
      setContent(sensorID, getGlobalID(uBin, vBin), landau_par);
    }

    /** Get content
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin)
     * @return value calibration value to store
     */
    std::pair<float, float> getContent(unsigned short sensorID, unsigned short globalID) const
    {
      auto mapIter = m_sensorCalibrationMap.find(sensorID);
      if (mapIter != m_sensorCalibrationMap.end()) {
        // Found sensor, return calibration value
        auto& calVec = mapIter->second;
        return calVec[globalID];
      }

      // Sensor not found, keep low profile and return default calibration value
      return m_landau_par;
    }

    /** Get content
     * @param sensorID unique ID of the sensor
     * @param uBin calibration bin along u side of sensor
     * @param vBin calibration bin along v side of sensor
     * @return value calibration value to store
     */
    std::pair<float, float> getContent(unsigned short sensorID, unsigned short uBin, unsigned short vBin) const
    {
      return getContent(sensorID, getGlobalID(uBin, vBin));
    }

    /** Return unordered_map with all PXD calibrations */
    const std::unordered_map<unsigned short, std::vector<std::pair<float, float>> >& getCalibrationMap() const {return m_sensorCalibrationMap;}

  private:

    /** Number of bins per sensor along u side */
    int m_nBinsU;

    /** Number of bins per sensor along v side */
    int m_nBinsV;

    /** MPV for map */
    std::pair<float, float> m_landau_par;

    /** Map for holding the calibrations for all PXD sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, std::vector<std::pair<float, float>> > m_sensorCalibrationMap;

    ClassDef(PXDClusterLandauMapPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
