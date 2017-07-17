/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_SHAPERDIGIT_H
#define SVD_SHAPERDIGIT_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/dataobjects/DigitBase.h>

#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <limits>

namespace Belle2 {

  /**
   * The SVD 6-digit class.
   *
   * The SVDShaperDigit is a set of 6 APV25 signal samples taken on a strip.
   */

  class SVDShaperDigit : public DigitBase {

  public:

    /** Number of APV samples stored */
    static const std::size_t c_nAPVSamples = 6;

    /** Types for array of samples received from DAQ.
     * An integer type is sufficient for storage, but getters will return array
     * of doubles suitable for computing.
     */
    typedef unsigned char APVRawSampleType;
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
     * @param time Time estimate from FADC
     * @param pipelineAddress APV pipeline address
     */
    template<typename T>
    SVDShaperDigit(VxdID sensorID, bool isU, short cellID,
                   T samples[c_nAPVSamples], char time = 0,
                   unsigned char pipelineAddress = 0):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_time(time),
      m_pipelineAddress(pipelineAddress)
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
     * @param time Time estimate from FADC
     * @param pipelineAddress APV pipeline address
     */
    template<typename T>
    SVDShaperDigit(VxdID sensorID, bool isU, short cellID,
                   T samples, char time = 0,
                   unsigned char pipelineAddress = 0):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_time(time),
      m_pipelineAddress(pipelineAddress)
    {
      std::transform(samples.begin(), samples.end(), m_samples.begin(),
                     [this](typename T::value_type x)->APVRawSampleType
      { return trimToSampleRange(x); }
                    );
    }

    /** Default constructor for the ROOT IO. */
    SVDShaperDigit() : SVDShaperDigit(
        0, true, 0, APVRawSamples( {0, 0, 0, 0, 0, 0}), 0, 0
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
    APVFloatSamples getSamples() const
    {
      APVFloatSamples returnSamples;
      std::transform(m_samples.begin(), m_samples.end(), returnSamples.begin(),
      [](APVRawSampleType x) { return static_cast<APVFloatSampleType>(x); });
      return returnSamples;
    }

    /** Get digit time estimate
     * @return digit time estimate from FADC
     */
    float getTime() const { return static_cast<float>(m_time); }

    /** Get pipeline address
     * @return APV pipeline address of the digit
     */
    unsigned short getPipelineAddress() const
    { return static_cast<unsigned short>(m_pipelineAddress);}

    /**
     * Convert a value to sample range.
     * @param value to be converted
     * @result APVRawSampleType representation of x
     */
    template<typename T> static SVDShaperDigit::APVRawSampleType trimToSampleRange(T x)
    {
      T trimmedX = std::min(
                     static_cast<T>(std::numeric_limits<SVDShaperDigit::APVRawSampleType>::max()),
                     std::max(
                       static_cast<T>(std::numeric_limits<SVDShaperDigit::APVRawSampleType>::lowest()),
                       x));
      return static_cast<SVDShaperDigit::APVRawSampleType>(trimmedX);
    }

    /** Display main parameters in this object */
    std::string toString() const
    {
      VxdID thisSensorID(m_sensorID);

      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID) << " strip: "
         << ((m_isU) ? "U-" : "V-") << m_cellID << " samples: ";
      std::copy(m_samples.begin(), m_samples.end(),
                std::ostream_iterator<APVRawSampleType>(os, " "));
      os << "FADC time: " << m_time << " PA: " << m_pipelineAddress << std::endl;
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
      const auto& bgSamples = dynamic_cast<const SVDShaperDigit*>(bg)->getSamples();
      // Add background samples to the digit's and trim back to range
      std::transform(m_samples.begin(), m_samples.end(), bgSamples.begin(),
                     m_samples.begin(),
                     [this](APVRawSampleType x, APVFloatSampleType y)->APVRawSampleType
      { return trimToSampleRange(x + y); }
                    );
      return DigitBase::c_DontAppend;
    }

  private:

    VxdID::baseType m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if U, false if V. */
    short m_cellID;            /**< Strip coordinate in pitch units. */
    APVRawSamples m_samples;      /**< 6 APV signals from the strip. */
    char m_time;              /**< digit time estimate from the FADC, in ns */
    unsigned char m_pipelineAddress;   /**< APV pipeline addressC */

    ClassDef(SVDShaperDigit, 1)

  }; // class SVDShaperDigit




} // end namespace Belle2

#endif // SVD_SHAPERDIGIT_H
