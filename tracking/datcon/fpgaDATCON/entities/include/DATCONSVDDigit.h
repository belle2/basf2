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

#include <cstdint>
#include <sstream>
#include <algorithm>
#include <limits>

namespace Belle2 {

  /**
  * The DATCONSVDDigit class.
  *
  * This class is a simplified version of the SVDShaperDigit class.
  * It is used for the DATCON simulation, as DATCON has less information of the SVD hits
  * available compared to the usual SVDShaperDigits.
  * The DATCONSVDDigit holds a set of 6 raw APV25 signal samples taken on a strip.
  */

  class DATCONSVDDigit {

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
    DATCONSVDDigit(VxdID sensorID, bool isU, short cellID,
                   APVFloatSampleType samples[c_nAPVSamples]):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_totalCharge(0), m_maxSampleCharge(0), m_maxSampleIndex(0)
    {
      std::transform(samples, samples + c_nAPVSamples, m_samples.begin(),
                     [this](APVFloatSampleType x)->APVRawSampleType { return trimToSampleRange(x); }
                    );
    }

    /** Constructor using a stl container of samples.
    * @param sensorID Sensor VXD ID.
    * @param isU True if u strip, false if v.
    * @param cellID Strip ID.
    * @param samples std::array of 6 APV raw samples.
    */
    DATCONSVDDigit(VxdID sensorID, bool isU, short cellID, APVFloatSamples samples) :
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_totalCharge(0), m_maxSampleCharge(0), m_maxSampleIndex(0)
    {
      std::transform(samples.begin(), samples.end(), m_samples.begin(),
                     [](typename APVFloatSamples::value_type x)->APVRawSampleType
      { return trimToSampleRange(x); }
                    );
    }

    /** Default constructor for the ROOT IO. */
    // cppcheck does not recognize initialization through other constructor
    // cppcheck-suppress uninitMemberVar
    DATCONSVDDigit() : DATCONSVDDigit(
        0, true, 0, APVFloatSamples( {{0, 0, 0, 0, 0, 0}})
    )
    {}

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
    template<typename T> static DATCONSVDDigit::APVRawSampleType trimToSampleRange(T x)
    {
      T trimmedX = std::min(
                     static_cast<T>(std::numeric_limits<DATCONSVDDigit::APVRawSampleType>::max()),
                     std::max(
                       static_cast<T>(std::numeric_limits<DATCONSVDDigit::APVRawSampleType>::lowest()),
                       x));
      return static_cast<DATCONSVDDigit::APVRawSampleType>(trimmedX);
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
    unsigned short m_totalCharge;     /**< Total charge of this DATCONSVDDigit */
    unsigned short m_maxSampleCharge; /**< Charge of sample max */
    unsigned short m_maxSampleIndex;  /**< Index of charge of sample max */

  }; // class DATCONSVDDigit

} // end namespace Belle2
