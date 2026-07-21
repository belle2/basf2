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

namespace Belle2 {

  /** The payload telling which charge threshold to use for PXD pixel
   */

  class PXDPixelThresholdPar: public TObject {
  public:
    /** Structure to hold set of single pixels indexed by their unique id (unsigned int), stored in hash table,
     * together with corresponding charge threshold
     */
    typedef std::unordered_map<unsigned int, unsigned short> SinglePixelThresholdsSet;

    /** Default constructor */
    PXDPixelThresholdPar(unsigned short defaultThr = 7) : m_defaultThreshold(defaultThr), m_MapSingleThresholds() {}

    /** Destructor */
    ~PXDPixelThresholdPar() {}

    /** Set threshold for single pixel
     *
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of single pixel to set threshold
     * @param pixThr charge threshold of the single pixel
     */
    void setSinglePixelThreshold(unsigned short sensorID, unsigned int pixID, unsigned short pixThr = 255)
    {
      auto mapIterSingles = m_MapSingleThresholds.find(sensorID);
      if (mapIterSingles != m_MapSingleThresholds.end()) {
        // Already some single pixel thresholds on sensor
        auto& singles = mapIterSingles->second;
        singles.insert(std::pair <unsigned int, unsigned short>(pixID, pixThr));
      } else {
        // Create an empty set of thresholds for single pixels
        PXDPixelThresholdPar::SinglePixelThresholdsSet singleThresholds;
        // pixID will be used to generate hash in unordered_map for quick access
        singleThresholds.insert(std::pair <unsigned int, unsigned short>(pixID, pixThr));
        m_MapSingleThresholds[sensorID] = singleThresholds;
      }
    }

    /** Check whether a pixel on a given sensor is OK or not and get threshold.
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of single pixel to set threshold
     * @return charge threshold of the single pixel.
     */
    unsigned short getPixelThreshold(unsigned short sensorID, unsigned int pixID) const
    {
      auto singleSensorThresholds = m_MapSingleThresholds.find(sensorID);
      if (singleSensorThresholds != m_MapSingleThresholds.end()) {
        // Found some masked single pixels on sensor
        auto& singleThresholds = singleSensorThresholds->second;
        // Look if this is a single masked pixel
        auto singleThr = singleThresholds.find(pixID);
        if (singleThr != singleThresholds.end()) {
          return singleThr->second;
        }
      }
      // Pixel not found in the threshold map
      return m_defaultThreshold;
    }

    /** Return unordered_map with all masked single pixels in PXD. */
    const std::unordered_map<unsigned short, SinglePixelThresholdsSet>& getPixelThresholdMap() const {return m_MapSingleThresholds;}

  private:

    /** Default value for charge threshold. Charge > Thr */
    unsigned short m_defaultThreshold;

    /** Structure holding sets of masked single pixels for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, SinglePixelThresholdsSet> m_MapSingleThresholds;

    ClassDef(PXDPixelThresholdPar, 1);  /**< ClassDef */
  };
} // end of namespace Belle2
