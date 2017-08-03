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

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/logging/Logger.h>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * SVD calibrations from the local runs providing the
   * constants needed to calibrate the SVDShaperDigit: the charge,
   * the ADC counts, the peaking time and the width.
   *
   * Currently the returned values are the default ones and they
   * are not read from any file.
   *
   */
  class SVDShapePulseCalibrations : public TObject {
  public:
    /** Constructor, no input argument is required */
    SVDShapePulseCalibrations()
    {}

    /** Return the charge collected on a specific
     * strip, given the number of ADC counts.
     * Currently it returns a default value for every strip,
     * assuming a unitary ADC pulse.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     * @param pulseADC : The ADC-pulse height is also required
     * as input argument, otherwise the default value is 1.
     *
     * Output: float corresponding to the charge [e] converted
     * from the read ADC pulse.
     */
    float getChargeFromADC(VxdID , bool , unsigned char,  unsigned char pulseADC)
    {
      return (float) pulseADC * m_gain;
    }

    /** Return an integer corresponding to the ADC pulse
     * height per strip, provided the charge [e] collected
     * on that strip.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     * @param charge: the charge in units [e] is also
     * required as input argument, otherwise the default value
     * is 22500.
     *
     * Output: an integer number representing the ADC pulse height
     * for the correponding input charge, on the given strip.
     */
    // it was previously defined as unsigned char, but not working
    int getADCFromCharge(VxdID , bool, unsigned char, float charge)
    {
      return charge / m_gain;
    }

    /** Return the peaking time of the strip.
     *
     * Input:
     * @param sensorID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: a float number corresponding to the peaking time
     */

    float getTau(VxdID , bool, unsigned char)
    {
      return 50;

    }

    /** Return the width of the pulse shape for a given strip.
     *
     * Input:
     * @param sensorID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: a float number corresponding to the pulse width in ns.
     */

    float getWidth(VxdID , bool, unsigned char)
    {
      return 120;

    }


  private:

    /** A number providing the conversion constant for the gain:
     * gain*pulseADC = charge [e]
     * charge/gain = pulse height [ADC counts]
     */
    const float m_gain = 22500. / 60.;
    ClassDef(SVDShapePulseCalibrations, 0);
  };
}

