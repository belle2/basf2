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
    typedef short int APVRawSampleType;
    typedef std::array<APVRawSampleType, c_nAPVSamples> APVRawSamples;

    /** Types for array of samples for processing.
     */
    typedef short int APVFloatSampleType;
    typedef std::array<APVFloatSampleType, c_nAPVSamples> APVFloatSamples;

    /** Useful Constructor.
     * @param sensorID Sensor VXD ID.
     * @param isU True if v strip, false if v.
     * @param cellID Strip ID.
     * @param samples std::array of 6 APV raw samples.
     * @param time Time estimate from FADC
     * @param timeError Time error estimate from FADC
     */
    SVDShaperDigit(VxdID sensorID, bool isU, short cellID, const APVRawSamples& samples,
                   float time = 0.0, float timeError = 100.0):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID), m_time(time), m_timeError(timeError)
    {
      std::copy(samples.begin(), samples.end(), m_samples.begin());
    }

    /** Default constructor for the ROOT IO. */
    SVDShaperDigit() : SVDShaperDigit(0, true, 0, {{0, 0, 0, 0, 0, 0}}, 0.0, 100.0)
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
    const APVFloatSamples& getSamples() const
    {
      static APVFloatSamples returnSamples;
      std::transform(m_samples.begin(), m_samples.end(), returnSamples.begin(),
      [](APVRawSampleType x) { return static_cast<APVFloatSampleType>(x); });
      return returnSamples;
    }

    /** Get digit time estimate
     * @return digit time estimate from FADC
     */
    float getTime() const { return m_time; }

    /** Get error of digit time estimate
     * @return error of digit time estimate
     */
    float getTimeError() const { return m_timeError; }

    /** Display main parameters in this object */
    std::string print() const
    {
      VxdID thisSensorID(m_sensorID);

      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID) << " strip: "
         << ((m_isU) ? "U-" : "V-") << m_cellID << " samples: ";
      std::copy(m_samples.begin(), m_samples.end(),
                std::ostream_iterator<APVRawSampleType>(os, " "));
      os << "Time: " << m_time << " +/- " << m_timeError << std::endl;
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
      std::transform(m_samples.begin(), m_samples.end(), bgSamples.begin(),
                     m_samples.begin(), std::plus<APVRawSampleType>());
      return DigitBase::c_DontAppend;
    }

  private:

    VxdID::baseType m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if U, false if V. */
    short m_cellID;            /**< Strip coordinate in pitch units. */
    APVRawSamples m_samples;      /**< 6 APV signals from the strip. */
    float m_time;              /**< digit time estimate from the FADC */
    float m_timeError;         /**< digit time error estiamte from the FADC */

    ClassDef(SVDShaperDigit, 1)

  }; // class SVDShaperDigit


} // end namespace Belle2

#endif // SVD_SHAPERDIGIT_H
