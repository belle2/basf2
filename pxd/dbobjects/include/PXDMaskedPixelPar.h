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
#include <unordered_set>


namespace Belle2 {

  /** The payload telling which PXD pixel to mask (ignore)
   */

  class PXDMaskedPixelPar: public TObject {
  public:
    /** Structure to hold set of masked single pixels indexed by their unique id (unsigned int), stored in hash table */
    typedef std::unordered_set< unsigned int> MaskedSinglePixelsSet;

    /** Default constructor */
    PXDMaskedPixelPar() : m_MapSingles() {}
    /** Destructor */
    ~PXDMaskedPixelPar() {}

    /** Mask single pixel
     *
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of single pixel to mask
     */
    void maskSinglePixel(unsigned short sensorID, unsigned int pixID)
    {
      auto mapIterSingles = m_MapSingles.find(sensorID);
      if (mapIterSingles != m_MapSingles.end()) {
        // Already some masked single pixels on sensor
        auto& singles = mapIterSingles->second;
        // Only add pixel, if it is not already in
        if (singles.find(pixID) == singles.end())
          singles.insert(pixID);
      } else {
        // Create an empty set of masked single pixels
        PXDMaskedPixelPar::MaskedSinglePixelsSet singles;
        // pixID will be used to generate hash in unordered_set for quick access
        singles.insert(pixID);
        m_MapSingles[sensorID] = singles;
      }
    }

    /** Check whether a pixel on a given sensor is OK or not.
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of single pixel to mask
     * @return true if pixel is not masked, otherwise false.
     */
    bool pixelOK(unsigned short sensorID, unsigned int pixID) const
    {
      auto mapIterSingles = m_MapSingles.find(sensorID);
      if (mapIterSingles != m_MapSingles.end()) {
        // Found some masked single pixels on sensor
        auto singles = mapIterSingles->second;
        // Look if this is a single masked pixel
        if (singles.find(pixID) != singles.end())
          return false;
      }
      // Pixel not found in the mask => pixel OK
      return true;
    }

    /** Return unordered_map with all masked single pixels in PXD. */
    const std::unordered_map<unsigned short, MaskedSinglePixelsSet>& getMaskedPixelMap() const {return m_MapSingles;}

  private:

    /** Structure holding sets of masked single pixels for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, MaskedSinglePixelsSet> m_MapSingles;

    ClassDef(PXDMaskedPixelPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
