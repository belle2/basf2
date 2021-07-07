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

  /** The payload class for PXD gain corrections
   *
   *  The payload class stores the gain correction values
   *  for PXD sensors on a grid nBinsU x nBinsV. The values
   *  are stored as dimensionless corrections factors. The
   *  default value is 1.0.
   *
   *  The baseline electron to ADU conversion is given as
   *
   *  1) baseline_eToADU = baseline_ADCUnit / baseline_Gq
   *
   *  and depends on the unit of the ADC channel and the Gq of the
   *  DEPFET transistor. These basline numbers are what is applied
   *  in the PXDDigitizer.
   *
   *  The gain corrected conversion factor is
   *
   *  2) corrected_eToADU  = baseline_eToADU / gain_correction
   *
   *  Variations inside a 2D bin will be averaged over. The Granularity
   *  of the grid can be adjusted by the user.
   */

  class PXDGainMapPar: public TObject {
  public:

    /** Default constructor */
    PXDGainMapPar(int nBinsU = 4, int nBinsV = 6, float defaultValue = 1.0) : m_nBinsU(nBinsU), m_nBinsV(nBinsV),
      m_defaultValue(defaultValue), m_sensorCalibrationMap() {}

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
     * @param value calibration value to store
     */
    void setContent(unsigned short sensorID, unsigned short globalID, float value)
    {
      auto mapIter = m_sensorCalibrationMap.find(sensorID);
      if (mapIter != m_sensorCalibrationMap.end()) {
        // Already some values stored
        auto& calVec = mapIter->second;
        // Set the value
        calVec[globalID] = value;
      } else {
        // Create a fresh calibration vector
        std::vector<float> calVec(m_nBinsU * m_nBinsV, m_defaultValue);
        // Set the value
        calVec[globalID] = value;
        // Add vector to map
        m_sensorCalibrationMap[sensorID] = calVec;
      }
    }

    /** Set map content
     *
     * @param sensorID unique ID of the sensor
     * @param uBin calibration bin along u side of sensor
     * @param vBin calibration bin along v side of sensor
     * @param value calibration value to store
     */
    void setContent(unsigned short sensorID, unsigned short uBin, unsigned short vBin, float value)
    {
      setContent(sensorID, getGlobalID(uBin, vBin), value);
    }

    /** Get content
     * @param sensorID unique ID of the sensor
     * @param globalID unique ID for part of sensor (uBin,vBin)
     * @return value calibration value to store
     */
    float getContent(unsigned short sensorID, unsigned short globalID) const
    {
      auto mapIter = m_sensorCalibrationMap.find(sensorID);
      if (mapIter != m_sensorCalibrationMap.end()) {
        // Found sensor, return calibration value
        auto& calVec = mapIter->second;
        return calVec[globalID];
      }
      // Sensor not found, keep low profile and return default calibration value
      return m_defaultValue;
    }

    /** Get content
     * @param sensorID unique ID of the sensor
     * @param uBin calibration bin along u side of sensor
     * @param vBin calibration bin along v side of sensor
     * @return value calibration value to store
     */
    float getContent(unsigned short sensorID, unsigned short uBin, unsigned short vBin) const
    {
      return getContent(sensorID, getGlobalID(uBin, vBin));
    }

    /** Return unordered_map with all PXD calibrations */
    const std::unordered_map<unsigned short, std::vector<float> >& getCalibrationMap() const {return m_sensorCalibrationMap;}

  private:

    /** Number of bins per sensor along u side */
    int m_nBinsU;

    /** Number of bins per sensor along v side */
    int m_nBinsV;

    /** Default value for map */
    float m_defaultValue;

    /** Map for holding the calibrations for all PXD sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, std::vector<float> > m_sensorCalibrationMap;

    ClassDef(PXDGainMapPar, 3);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
