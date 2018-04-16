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
#include <svd/dataobjects/SVDModeByte.h>
#include <framework/dataobjects/DigitBase.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <limits>

namespace Belle2 {

  /**
  * The SVD ShaperDigit class.
  *
  * The DATCONSVDDigit holds a set of 6 raw APV25 signal samples,
  * zero-padded in 3-sample mode) taken on a strip. It also holds
  * DAQ mode (3 or 6 samples) and trigger time information in an
  * SVDModeByte structure, and time fit from FADC (when available).
  */

//     class DATCONSVDDigit : public DigitBase {
  class DATCONSVDDigit : public RelationsObject {

  public:

    /** Number of APV samples stored */
    static const std::size_t c_nAPVSamples = 6;

    /** Types for array of samples received from DAQ.
    * An integer type is sufficient for storage, but getters will return array
    * of doubles suitable for computing.
    */
    typedef uint8_t APVRawSampleType;
    typedef std::array<APVRawSampleType, c_nAPVSamples> APVRawSamples;

    /** Types for array of samples for processing.
    */
    typedef float APVFloatSampleType;
    typedef std::array<APVFloatSampleType, c_nAPVSamples> APVFloatSamples;

    /** Constructor using c-array of samples.
    * @param sensorID Sensor VXD ID.
    * @param isU True if u strip, false if v.
    * @param cellID Strip ID.
    * @param samples std::array of 6 APV raw samples.
    * @param mode SVDModeByte structure, packed trigger time bin and DAQ
    * mode.
    */
    template<typename T>
    DATCONSVDDigit(VxdID sensorID, bool isU, short cellID,
                   T samples[c_nAPVSamples],
                   SVDModeByte mode = SVDModeByte()):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_mode(mode.getID())
      //m_totalCharge(0), m_maxSampleIndex(0)
    {
      m_totalCharge = 0;
      m_maxSampleIndex = 0;
      std::transform(samples, samples + c_nAPVSamples, m_samples.begin(),
                     [this](T x)->APVRawSampleType { return trimToSampleRange(x); }
                    );
    }

    /** Constructor using a stl container of samples.
    * @param sensorID Sensor VXD ID.
    * @param isU True if u strip, false if v.
    * @param cellID Strip ID.
    * @param samples std::array of 6 APV raw samples.
    * @param mode SVDModeByte structure, packed trigger time bin and DAQ
    * mode.
    */
    template<typename T>
    DATCONSVDDigit(VxdID sensorID, bool isU, short cellID, T samples,
                   SVDModeByte mode = SVDModeByte()) :
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID),
      m_mode(mode.getID())//, m_totalCharge(0), m_maxSampleIndex(0)
    {
      m_totalCharge = 0;
      m_maxSampleIndex = 0;
      std::transform(samples.begin(), samples.end(), m_samples.begin(),
                     [this](typename T::value_type x)->APVRawSampleType
      { return trimToSampleRange(x); }
                    );
    }

    /** Default constructor for the ROOT IO. */
    DATCONSVDDigit() : DATCONSVDDigit(
        0, true, 0, APVRawSamples( {{0, 0, 0, 0, 0, 0}})
    )
    { }

    /** Get the sensor ID.
    * @return ID of the sensor.
    */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get raw sensor ID.
    * For use in Python
    * @return basetype ID of the sensor.
    */
    VxdID::baseType getRawSensorID() const { return m_sensorID; }

    /** Get strip direction.
    * @return true if u, false if v.
    */
    bool isUStrip() const { return m_isU; }

    /** Get strip ID.
    * @return ID of the strip.
    */
    short int getCellID() const { return m_cellID; }

    /** Get arrray of samples.
    * @return std::array of 6 APV25 samples.
    */
    APVFloatSamples getFloatSamples() const
    {
      APVFloatSamples returnSamples;
      std::transform(m_samples.begin(), m_samples.end(), returnSamples.begin(),
      [](APVRawSampleType x) { return static_cast<APVFloatSampleType>(x); });
      return returnSamples;
    }

    /** Get arrray of samples.
    * @return std::array of 6 APV25 samples.
    */
    APVRawSamples getRawSamples() const
    {
      return m_samples;
    }

    inline unsigned short getTotalCharge()
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

    inline unsigned short getMaxSampleCharge()
    {
      unsigned short maxCharge = 0;
      for (unsigned int i = 0; i < c_nAPVSamples; i++) {
        if (m_samples[i] > maxCharge) {
          maxCharge = m_samples[i];
        }
      }
      return maxCharge;
    }


    inline unsigned short getMaxSampleIndex()
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


    /** Get the SVDMOdeByte object containing information on trigger FADCTime and DAQ mode.
    * @return the SVDModeByte object of the digit
    */
    SVDModeByte getModeByte() const { return m_mode; }

    /**
    * Convert a value to sample range.
    * @param value to be converted
    * @result APVRawSampleType representation of x
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

    /** Display main parameters in this object */
    std::string toString() const
    {
      VxdID thisSensorID(m_sensorID);
      SVDModeByte thisMode(m_mode);

      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID) << " strip: "
         << ((m_isU) ? "U-" : "V-") << m_cellID << " samples: ";
      std::copy(m_samples.begin(), m_samples.end(),
                std::ostream_iterator<APVRawSampleType>(os, " "));
      //       os << "FADC time: " << m_FADCTime << " " << thisMode << std::endl;
      os << thisMode << std::endl;
      return os.str();
    }

    /**
    * Implementation of base class function.
    * Enables BG overlay module to identify uniquely the physical channel of this
    * Digit.
    * @return unique channel ID, composed of VxdID (1 - 16), strip side (17), and
    * strip number (18-28)
    */
    unsigned int getUniqueChannelID() const
    { return m_cellID + ((m_isU ? 1 : 0) << 11) + (m_sensorID << 12); }

    /**
    * Implementation of base class function.
    * Addition is always possible, so we always return successful merge.
    * Pile-up method.
    * @param bg beam background digit
    * @return append status
    */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg)
    {
      // Don't modify and don't append when bg points nowhere.
      if (!bg) return DigitBase::c_DontAppend;
      const auto& bgSamples = dynamic_cast<const DATCONSVDDigit*>(bg)->getFloatSamples();
      // Add background samples to the digit's and trim back to range
      std::transform(m_samples.begin(), m_samples.end(), bgSamples.begin(),
                     m_samples.begin(),
                     [this](APVRawSampleType x, APVFloatSampleType y)->APVRawSampleType
      { return trimToSampleRange(x + y); }
                    );
      // FIXME: Reset FADC time flag in mode byte.
      return DigitBase::c_DontAppend;
    }

    /**
    *
    * @param
    * @return append status
    */
    bool operator < (const DATCONSVDDigit&   x)const
    {

      if (getSensorID() != x.getSensorID())
        return getSensorID() < x. getSensorID();
      if (isUStrip() != x.isUStrip())
        return isUStrip();
      else
        return getCellID() < x.getCellID();
    }


    void setSensorID(VxdID sensorid) { m_sensorID = sensorid; }

    void setUStrip(bool isU) { m_isU = isU; }

    void setCellID(short cellID) { m_cellID = cellID; }

    void setAPVRawSamples(APVFloatSamples apvInputSamples)
    {
      //       m_samples = apvSamples;
      std::transform(apvInputSamples.begin(), apvInputSamples.end(), m_samples.begin(),
      [](APVFloatSampleType x) { return static_cast<APVRawSampleType>(x); });
    }


    void setSVDModeByte(SVDModeByte mode) { m_mode = mode; }

  private:

    VxdID::baseType m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU; /**< True if U, false if V. */
    short m_cellID; /**< Strip coordinate in pitch units. */
    APVRawSamples m_samples; /**< 6 APV signals from the strip. */
    SVDModeByte::baseType m_mode; /**< Mode byte, trigger FADCTime + DAQ mode */
    unsigned short m_maxSampleIndex; /**< Index of charge of sample max */
    unsigned short m_maxSampleCharge; /** Charge of sample max */
    unsigned short m_totalCharge; /**< Total charge of this DATCONShaperDigit */

    ClassDef(DATCONSVDDigit, 1)

  }; // class DATCONSVDDigit

} // end namespace Belle2
