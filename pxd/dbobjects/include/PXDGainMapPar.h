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
   *  Gain corrections are stored by the PXD sensorID and the chipID.
   *  There are chipIDs for the 24 different combinations of DCD
   *  and SWB steering ASICs.
   */

  class PXDGainMapPar: public TObject {
  public:

    /** Default constructor */
    PXDGainMapPar() : m_sensorGainMap() {}
    /** Destructor */
    ~PXDGainMapPar() {}

    /** Set gain correction
     *
     * @param sensorID unique ID of the sensor
     * @param chipID unique ID for DCD/SWB chip pair
     * @param gain relative gain correction
     */
    void setGainCorrection(unsigned short sensorID, unsigned short chipID, float gain)
    {
      auto mapIter = m_sensorGainMap.find(sensorID);
      if (mapIter != m_sensorGainMap.end()) {
        // Already some gains stored
        auto& gainVec = mapIter->second;
        // Set the gain
        gainVec[chipID] = gain;
      } else {
        // Create a fresh gain vector with trivial corrections
        std::vector<float> gainVec(24, 1.0);
        // Set the gain
        gainVec[chipID] = gain;
        // Add gain vector to map
        m_sensorGainMap[sensorID] = gainVec;
      }
    }

    /** Get gain correction
     * @param sensorID unique ID of the sensor
     * @param chipID unique ID for DCD/SWB chip pair
     * @return gain
     */
    float getGainCorrection(unsigned short sensorID, unsigned short chipID) const
    {
      auto mapIter = m_sensorGainMap.find(sensorID);
      if (mapIter != m_sensorGainMap.end()) {
        // Found sensor, return gain correction
        auto& gainVec = mapIter->second;
        return gainVec[chipID];
      }
      // Sensor not found, keep low profile and return trivial correction
      return 1.0;
    }

    /** Return unordered_map with all PXD gain corrections */
    const std::unordered_map<unsigned short, std::vector<float> >& getSensorGainMap() const {return m_sensorGainMap;}

  private:

    /** Map for holding the gain corrections for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, std::vector<float> > m_sensorGainMap;

    ClassDef(PXDGainMapPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
