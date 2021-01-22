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

#include <vxd/dataobjects/VxdID.h>
#include <framework/dataobjects/DigitBase.h>

#include <cstdint>
#include <sstream>
#include <algorithm>
#include <limits>

namespace Belle2 {

  /**
  * The DATCONSVDDigit2 class.
  *
  * This class is a simplified version of the SVDShaperDigit class.
  * It is used for the DATCON simulation, as DATCON has less information of the SVD hits
  * available compared to the usual SVDShaperDigits.
  * The DATCONSVDDigit2 holds a set of 6 raw APV25 signal samples taken on a strip.
  */

  class DATCONSVDDigit2 : public DigitBase {
//   class DATCONSVDDigit2 : public RelationsObject {

  public:

    /** Number of APV samples stored */
    static const std::size_t c_nAPVSamples = 6;

    /** Type of samples received from DAQ. */
    typedef uint8_t APVRawSampleType;
    /** Type for array of samples received from DAQ. */
    typedef std::array<APVRawSampleType, c_nAPVSamples> APVRawSamples;

    /** Types of samples for processing. */
    typedef float APVFloatSampleType;
    /** Types for array of samples for processing. */
    typedef std::array<APVFloatSampleType, c_nAPVSamples> APVFloatSamples;

    /** Constructor using c-array of samples.
    * @param sensorID Sensor VXD ID.
    * @param isU True if u strip, false if v.
    * @param cellID Strip ID.
    * @param samples std::array of 6 APV raw samples.
    */
    template<typename T>
    DATCONSVDDigit2(VxdID sensorID, bool isU, short cellID,
                    T samples[c_nAPVSamples]):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_totalCharge(0), m_maxSampleCharge(0), m_maxSampleIndex(0)
    {
      std::transform(samples, samples + c_nAPVSamples, m_samples.begin(),
                     [this](T x)->APVRawSampleType { return trimToSampleRange(x); }
                    );
    }

    /** Constructor using a stl container of samples.
    * @param sensorID Sensor VXD ID.
    * @param isU True if u strip, false if v.
    * @param cellID Strip ID.
    * @param samples std::array of 6 APV raw samples.
    */
    template<typename T>
    DATCONSVDDigit2(VxdID sensorID, bool isU, short cellID, T samples) :
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_totalCharge(0), m_maxSampleCharge(0), m_maxSampleIndex(0)
    {
      std::transform(samples.begin(), samples.end(), m_samples.begin(),
                     [](typename T::value_type x)->APVRawSampleType
      { return trimToSampleRange(x); }
                    );
    }

    /** Default constructor for the ROOT IO. */
    // cppcheck does not recognize initialization through other constructor
    // cppcheck-suppress uninitMemberVar
    DATCONSVDDigit2() : DATCONSVDDigit2(
        0, true, 0, APVRawSamples( {{0, 0, 0, 0, 0, 0}})
    )
    {}

    /**
    * Implementation of base class function.
    * Enables BG overlay module to identify uniquely the physical channel of this
    * Digit.
    * @return unique channel ID, composed of VxdID (1 - 16), strip side (17), and
    * strip number (18-28)
    */
    unsigned int getUniqueChannelID() const override
    { return m_cellID + ((m_isU ? 1 : 0) << 11) + (m_sensorID << 12); }

    /**
    * Implementation of base class function.
    * Addition is always possible, so we always return successful merge.
    * Pile-up method.
    * @param bg beam background digit
    * @return append status
    */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg) override
    {
      // Don't modify and don't append when bg points nowhere.
      if (!bg) return DigitBase::c_DontAppend;
//       const auto& bgSamples = dynamic_cast<const DATCONSVDDigit2*>(bg)->getFloatSamples();
//       // Add background samples to the digit's and trim back to range
//       std::transform(m_samples.begin(), m_samples.end(), bgSamples.begin(),
//                      m_samples.begin(),
//                      [](APVRawSampleType x, APVFloatSampleType y)->APVRawSampleType
//       { return trimToSampleRange(x + y); }
//                     );
//       // FIXME: Reset FADC time flag in mode byte.
      return DigitBase::c_DontAppend;
    }

    /** Getter for the sensor ID. */
    VxdID getSensorID() const { return m_sensorID; }

    /** Getter for the raw sensor ID. */
    VxdID::baseType getRawSensorID() const { return m_sensorID; }

    /** Getter for the strip direction (u or v) */
    bool isUStrip() const { return m_isU; }

    /** Getter for the strip ID. */
    short int getCellID() const { return m_cellID; }

    /** Get float-array of 6 APV25 samples.  */
    APVFloatSamples getFloatSamples() const
    {
      APVFloatSamples returnSamples;
      std::transform(m_samples.begin(), m_samples.end(), returnSamples.begin(),
      [](APVRawSampleType x) { return static_cast<APVFloatSampleType>(x); });
      return returnSamples;
    }

    /** Get int-array of of 6 APV25 samples. */
    APVRawSamples getRawSamples() const
    {
      return m_samples;
    }

    /** Getter for the total charge of the array in ADUs. */
    unsigned short getTotalCharge()
    {
      if (m_totalCharge > 0)
        return m_totalCharge;
      else {
        m_totalCharge = 0;
        for (unsigned int i = 0; i < c_nAPVSamples; i++) {
          m_totalCharge += m_samples[i];
        }
      }
      return m_totalCharge;
    }

    /** Getter for the charge of the biggest sample of the array in ADUs. */
    unsigned short getMaxSampleCharge()
    {
      unsigned short maxCharge = 0;
      for (unsigned int i = 0; i < c_nAPVSamples; i++) {
        if (m_samples[i] > maxCharge) {
          maxCharge = m_samples[i];
        }
      }
      return maxCharge;
    }

    /** Getter for the index of the biggest sample inside the cluster (0...6) */
    unsigned short getMaxSampleIndex()
    {
      if (m_maxSampleIndex > 0)
        return m_maxSampleIndex;
      else {
        unsigned short maxCharge = 0;
        for (unsigned int i = 0; i < c_nAPVSamples; i++) {
          if (m_samples[i] > maxCharge) {
            maxCharge = m_samples[i];
            m_maxSampleIndex = i;
          }
        }
      }
      return m_maxSampleIndex;
    }

    /**
    * Convert a value to sample range.
    * @param x value to be converted
    * @return  APVRawSampleType representation of x
    */
    template<typename T> static DATCONSVDDigit2::APVRawSampleType trimToSampleRange(T x)
    {
      T trimmedX = std::min(
                     static_cast<T>(std::numeric_limits<DATCONSVDDigit2::APVRawSampleType>::max()),
                     std::max(
                       static_cast<T>(std::numeric_limits<DATCONSVDDigit2::APVRawSampleType>::lowest()),
                       x));
      return static_cast<DATCONSVDDigit2::APVRawSampleType>(trimmedX);
    }


    /** Setter for the sensorID. */
    void setSensorID(VxdID sensorid) { m_sensorID = sensorid; }

    /** Setter for the strip direction (u or v). */
    void setUStrip(bool isU) { m_isU = isU; }

    /** Setter for the stripID / cellID. */
    void setCellID(short cellID) { m_cellID = cellID; }

    /** Setter for the raw samples array. */
    void setAPVRawSamples(APVFloatSamples apvInputSamples)
    {
      //       m_samples = apvSamples;
      std::transform(apvInputSamples.begin(), apvInputSamples.end(), m_samples.begin(),
      [](APVFloatSampleType x) { return static_cast<APVRawSampleType>(x); });
    }

  private:

    VxdID::baseType m_sensorID;       /**< Compressed sensor identifier.*/
    bool m_isU;                       /**< True if U, false if V. */
    short m_cellID;                   /**< Strip coordinate in pitch units. */
    APVRawSamples m_samples;          /**< 6 APV signals from the strip. */
    unsigned short m_totalCharge;     /**< Total charge of this DATCONSVDDigit2 */
    unsigned short m_maxSampleCharge; /**< Charge of sample max */
    unsigned short m_maxSampleIndex;  /**< Index of charge of sample max */

    ClassDefOverride(DATCONSVDDigit2, 2);

  }; // class DATCONSVDDigit2

} // end namespace Belle2
