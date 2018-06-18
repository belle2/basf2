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
#include <svd/dbobjects/SVDCalibrationsScalar.h>
#include <svd/dbobjects/SVDStripCalAmp.h>
#include <svd/dbobjects/SVDTriggerBinDependentConstants.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
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
    static std::string calAmp_name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< SVDStripCalAmp > > t_calAmp_payload;
    static std::string time_name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_time_payload;
    static std::string bin_name;
    typedef SVDCalibrationsBase< SVDCalibrationsScalar< SVDTriggerBinDependentConstants > > t_bin_payload;

    /** Constructor, no input argument is required */
    SVDPulseShapeCalibrations()
      : m_calAmp_aDBObjPtr(calAmp_name)
      , m_time_aDBObjPtr(time_name)
      , m_bin_aDBObjPtr(bin_name)
    {
      m_calAmp_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDPulseShapeCalibrations: from now one we are using " <<
        this->m_calAmp_aDBObjPtr -> get_uniqueID()); });
    }

    /** Return the charge (number of electrons/holes) collected on a specific
     * strip, given the number of ADC counts.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     * @param pulseADC : The ADC-pulse height, a double between 0 and 255 (included)
     *
     * Output: float corresponding to the charge [e] converted
     * from the read ADC pulse.
     */
    inline double getChargeFromADC(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const double& pulseADC
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
     * @param charge: the charge in electrons
     *
     * Output: a float number corresponding to the peaking time
     */

    inline float getPeakTime(const VxdID& sensorID, const bool& isU,
                             const unsigned short& strip) const
    {
      return m_calAmp_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                                     sensorID.getSensorNumber(), m_calAmp_aDBObjPtr->sideIndex(isU),
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
      return m_calAmp_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                     m_calAmp_aDBObjPtr->sideIndex(isU), strip).pulseWidth;

    }

    /** Return the time shift to be applied to theCoG time estimator
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

    /** return the trigger bin dependent correction for the CoG time estimator
     *
     * Input:
     * @param sensorID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number - NOT USED
     *
     * Output: a float number corresponding to the time correction in ns.
     */
    inline float getTriggerBinDependentCorrection(const VxdID& sensorID, const bool& isU,
                                                  const unsigned short& strip, const int& bin) const
    {
      float correction = 0;
      if (bin == 0)
        correction = m_bin_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                          m_bin_aDBObjPtr->sideIndex(isU), strip).bin0;
      else if (bin == 1)
        correction = m_bin_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                          m_bin_aDBObjPtr->sideIndex(isU), strip).bin1;
      else if (bin == 2)
        correction = m_bin_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                          m_bin_aDBObjPtr->sideIndex(isU), strip).bin2;
      else if (bin == 3)
        correction = m_bin_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                          m_bin_aDBObjPtr->sideIndex(isU), strip).bin3;
      else
        B2WARNING("SVDPulseShapeCalibrations: you ar asking for a non existing trigger bin! Return 0.");

      return correction;
    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_calAmp_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid()
    {
      return m_calAmp_aDBObjPtr.isValid() &&
             m_time_aDBObjPtr.isValid() &&
             m_bin_aDBObjPtr.isValid();
    }

  private:

    /** Return the channel gain.
     * the gain is expressed in ADC counts / # electrons injected in the channel
     * That is:
     * pulseADC / gain = charge [e]
     * charge * gain = pulse height [ADC counts]
     */

    inline float getGain(const VxdID& sensorID, const bool& isU,
                         const unsigned short& strip) const
    {
      return m_calAmp_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
                                     m_calAmp_aDBObjPtr->sideIndex(isU), strip).gain ;

    }

  private:
    DBObjPtr< t_calAmp_payload > m_calAmp_aDBObjPtr;
    DBObjPtr< t_time_payload > m_time_aDBObjPtr;
    DBObjPtr< t_bin_payload > m_bin_aDBObjPtr;

  };
}

