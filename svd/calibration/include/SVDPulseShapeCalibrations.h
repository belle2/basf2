/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: Do not try to fry it with water. Use only olive oil.          *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsVector.h>
#include <svd/dbobjects/SVDStripCalAmp.h>
#include <framework/database/DBObjPtr.h>
#include <string>

#include <math.h>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * SVD calibrations from the local runs providing the
   * constants needed to calibrate the SVDShaperDigit: the charge,
   * the ADC counts, the peaking time and the width.
   *
   */
  class SVDPulseShapeCalibrations {
  public:
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< SVDStripCalAmp > > t_payload;
    static std::string time_name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_time_payload;

    /** Constructor, no input argument is required */
    SVDPulseShapeCalibrations()
      : m_aDBObjPtr(name)
      , m_time_aDBObjPtr(time_name)
    {}

    /** Return the charge (number of electrons/holes) collected on a specific
     * strip, given the number of ADC counts.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     * @param pulseADC : The ADC-pulse height is also required
     * as input argument.
     *
     * Output: float corresponding to the charge [e] converted
     * from the read ADC pulse.
     */
    inline double getChargeFromADC(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const unsigned char& pulseADC
    ) const
    {
      return  pulseADC / getGain(sensorID, isU, strip);
    }

    /** Return a signed long integer corresponding to the ADC pulse
     * height per strip, provided the charge [e] collected
     * on that strip.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     * @param charge: the charge in units [e] is also
     * required as input argument
     *
     * Output: an integer number representing the ADC pulse height
     * for the correponding input charge, on the given strip.
     * The output is capped at 255.
     */
    inline long int getADCFromCharge(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const double& charge) const
    {
      return roundl(charge * getGain(sensorID, isU, strip));
    }

    /** Return an unsigned 8 bit integer corresponding to the ADC
     * pulse height per strip, provided the charge [e] collected
     * on that strip.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     * @param charge: the charge in units [e] is also
     * required as input argument
     *
     * Output: an integer number representing the ADC pulse height
     * for the correponding input charge, on the given strip.
     * The output is capped at 255.
     */

    inline unsigned char getCappedADCFromCharge(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const double& charge) const
    {
      auto chargeLongInt = getADCFromCharge(sensorID, isU, strip, charge);
      if (chargeLongInt < 0)
        return 0;
      return chargeLongInt > 255 ? 255 : chargeLongInt;
    }

    /** Return the peaking time of the strip.
     * Input:
     * @param sensorID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: a float number corresponding to the peaking time
     */

    inline float getPeakTime(const VxdID& sensorID, const bool& isU,
                             const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                              strip).peakTime;
    }

    /** Return the width of the pulse shape for a given strip.
     * Since it is provided from local run measurements in
     * [APV clock/8] units, the correct value to be uploaded on the
     * central DB for the width in [ns], the conversion factor
     * [31.44ns/8] must be applied.
     * The payload already retrieves the converted width in [ns].
     *
     * Input:
     * @param sensorID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: a float number corresponding to the pulse width in ns.
     */

    inline float getWidth(const VxdID& sensorID, const bool& isU,
                          const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU), strip).pulseWidth;

    }

    /** Return the time shift to be applied to the basic time estimator
     * (weighted average of the time)
     *
     * Input:
     * @param sensorID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: a float number corresponding to the time shift width in ns.
     */
    inline float getTimeShiftCorrection(const VxdID& sensorID, const bool& isU,
                                        const unsigned short& strip) const
    {
      return m_time_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                   m_time_aDBObjPtr->sideIndex(isU), strip);

    }


  private:

    /** Return the channel gain.
     * the gain is expressed in ADC counts / # electrons injected in the channel
     * That is:
     * gain / pulseADC = charge [e]
     * charge * gain = pulse height [ADC counts]
     */

    inline float getGain(const VxdID& sensorID, const bool& isU,
                         const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU), strip).gain ;


    }

  private:
    DBObjPtr< t_payload > m_aDBObjPtr;
    DBObjPtr< t_time_payload > m_time_aDBObjPtr;

  };
}

