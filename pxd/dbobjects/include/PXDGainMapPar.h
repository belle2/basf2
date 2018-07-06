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

  /** The payload for PXD gain corrections
   *
   *  Gain corrections are dimensionless corrections for the
   *  nominal converion of ADC counts to the number of electron
   *  hole pairs in silicon.
   *
   *  The baseline electron to ADU conversion is given as
   *
   *  baseline_eToADU = ADCUnit / Gq
   *
   *  and depends on the unit of the ADC channel and the Gq of the
   *  DEPFET transistor. The gain corrected conversion factor is
   *
   *  corrected_eToADU  = baseline_eToADU / gain_correction
   *
   *  Gain corrections are stored by the PXD sensorID and the gainID.
   *  The gainID enumerate a rectangular regions on the sensor. Gain
   *  variations insied such a sensor region are not taken into account.
   */

  class PXDGainMapPar: public TObject {
  public:

    /** Default constructor */
    PXDGainMapPar(int nBinsU = 4, int nBinsV = 6) : m_nBinsU(nBinsU), m_nBinsV(nBinsV), m_sensorGainMap() {}

    /** Get number of corrections along u side
     */
    unsigned short getCorrectionsU() const { return m_nBinsU; }

    /** Get number of corrections along v side
     */
    unsigned short getCorrectionsV() const { return m_nBinsV; }

    /** Get unique id for gain correction
     */
    unsigned short getGainID(unsigned short uBin, unsigned short vBin) const { return uBin * m_nBinsV + vBin; }

    /** Set gain correction
     *
     * @param sensorID unique ID of the sensor
     * @param gainID unique ID for rectangular part of sensor
     * @param gain relative gain correction
     */
    void setGainCorrection(unsigned short sensorID, unsigned short gainID, float gain)
    {
      auto mapIter = m_sensorGainMap.find(sensorID);
      if (mapIter != m_sensorGainMap.end()) {
        // Already some gains stored
        auto& gainVec = mapIter->second;
        // Set the gain
        gainVec[gainID] = gain;
      } else {
        // Create a fresh gain vector with trivial corrections
        std::vector<float> gainVec(m_nBinsU * m_nBinsV, 1.0);
        // Set the gain
        gainVec[gainID] = gain;
        // Add gain vector to map
        m_sensorGainMap[sensorID] = gainVec;
      }
    }

    /** Set gain correction
     *
     * @param sensorID unique ID of the sensor
     * @param uBin part of the unique gain ID
     * @param vBin part of the unique gain ID
     * @param gain relative gain correction
     */
    void setGainCorrection(unsigned short sensorID, unsigned short uBin, unsigned short vBin, float gain)
    {
      unsigned short gainID =  uBin * m_nBinsV + vBin;
      setGainCorrection(sensorID, gainID, gain);
    }

    /** Get gain correction
     * @param sensorID unique ID of the sensor
     * @param gainID unique ID for rectangular part of sensor
     * @return gain
     */
    float getGainCorrection(unsigned short sensorID, unsigned short gainID) const
    {
      auto mapIter = m_sensorGainMap.find(sensorID);
      if (mapIter != m_sensorGainMap.end()) {
        // Found sensor, return gain correction
        auto& gainVec = mapIter->second;
        return gainVec[gainID];
      }
      // Sensor not found, keep low profile and return trivial correction
      return 1.0;
    }

    /** Get gain correction
     * @param sensorID unique ID of the sensor
     * @param uBin part of the unique gain ID
     * @param vBin part of the unique gain ID
     * @return gain
     */
    float getGainCorrection(unsigned short sensorID, unsigned short uBin, unsigned short vBin) const
    {
      unsigned short gainID =  uBin * m_nBinsV + vBin;
      return getGainCorrection(sensorID, gainID);
    }

    /** Return unordered_map with all PXD gain corrections */
    const std::unordered_map<unsigned short, std::vector<float> >& getSensorGainMap() const {return m_sensorGainMap;}

  private:

    /** Number of corrections per sensor along u side */
    int m_nBinsU;

    /** Number of corrections per sensor along v side */
    int m_nBinsV;

    /** Map for holding the gain corrections for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, std::vector<float> > m_sensorGainMap;

    ClassDef(PXDGainMapPar, 2);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
