/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <framework/logging/Logger.h>
#include <string>
#include <unordered_map>

namespace Belle2 {
  namespace SVD {

    class StripCalibrationMap {
    public:
      /** Structure holding the strip calibration data.
       * Each query returns the whole set of strip parameters.
       */
      typedef struct MaskNoisePeak {
        MaskNoisePeak(bool goodStrip, float noise, float calPeak,
                      short calWidth, float calTimeDelay):
          m_goodStrip(goodStrip), m_noise(noise), m_calPeak(calPeak),
          m_calWidth(calWidth), m_calTimeDelay(calTimeDelay) {};
        bool m_goodStrip;
        float m_noise;
        float m_calPeak;
        short m_calWidth;
        float m_calTimeDelay;
      } StripData;

      /** Constructor needs to know the online-to-offline map xml name,
       * and the source calibration filename. If the filenames are empty, a default
       * calibration map will be constructed using GeoCache data.
       * NB: Since GeoCache is used to construct default map, the class only
       * works inside a basf job that builds SVD geometry.
       * @param ooMapName Name of xml file containing the SVD online-to-
       * offline map.
       * @param calFileName Name of xml file with hardware test data.
       */
      StripCalibrationMap(const std::string& ooMapName, const std::string& calFileName)
      {
        // If no xml file or in case of error, use the default map.
        if (
          (ooMapName == "") || (calFileName == "")
          || (!readMapFromXml(ooMapName, calFileName))
        )
          constructDefaultMap();
      }

      /** Principal getter
       * @param sensorID ID of the sensor,
       * @param isU u-side strip?
       * @param stripNo strip number
       * @return const& of the StripData structure containing the required data. */
      const StripData& getStripData(VxdID sensorID, bool isU,
                                    short stripNo)
      {
        unsigned int uID = getUniqueChannelID(sensorID, isU, stripNo);
        auto findResult = m_stripData.find(uID);
        if (findResult != m_stripData.end())
          return findResult->second;
        else {
          B2WARNING("Calibration data NOT FOUND!!!" << std::endl <<
                    "sensor: " << sensorID << " side: " << (isU ? 1 : 0)
                    << " strip: " << stripNo);
          return c_noStripData;
        }
      }

      /** Get calibration charge level */
      int getCalibrationLevel() const { return m_calLevel; }

      /** Produce table of values */
      std::string toString();

    protected:

      /** Read calibration data from the hardware calibration xml file.
       * The ooMap file is needed to convert FADC/ADC/APV/strip tags to VxdID/side/strip.
       * @param ooMapName Name of xml file containing the SVD online-to-
       * offline map.
       * @param calFileName Name of xml file with hardware test data.
       */
      bool readMapFromXml(const std::string& ooMapName, const std::string& calFileName);
      /** Construct default calibration map using data from GeoCache. */
      void constructDefaultMap();

    private:

      const StripData c_noStripData = StripData({false, 100, 0.0, 100, 0.0});

      /** Unique strip ID
       * The ID is created by bit-shifted combination of sensor ID,
       * side and strip number. No decoding is needed for its designed
       * use. The method is identical to SVD*Digit's getUniqueChannelID()
       * method.
       * @param sensorID VxdID of the strip's sensor
       * @param isU is this u-side strip?
       * @param stripNo strip number.
       * @return unique strip ID, composed of VxdID (1 - 16),
       * strip side (17), and strip number (18-28).
      */
      unsigned int getUniqueChannelID(VxdID sensorID, bool isU,
                                      unsigned short stripNo) const
      { return stripNo + ((isU ? 1 : 0) << 11) + (sensorID << 12); }

      typedef std::unordered_map<unsigned int, StripData>  stripCalMap;

      // Data members
      int m_calLevel; /**< Corresponds to pulse charge used */
      stripCalMap m_stripData; /**< Map holding strip data */

    }; // class StripCalibrationMap
  } // namespace SVD
} // namespace Belle2
