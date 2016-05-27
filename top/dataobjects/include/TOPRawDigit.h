/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Class to store unpacked raw data (hits in feature-extraction format)
   * It provides also calculation of 50% CFD leading and falling times
   */

  class TOPRawDigit : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPRawDigit()
    {}

    /**
     * Usefull constructor
     * @param scrodID SCROD ID
     */
    TOPRawDigit(unsigned short scrodID): m_scrodID(scrodID)
    {}

    /**
     * Sets carrier board number
     * @param carrier number
     */
    void setCarrierNumber(unsigned short carrier) {m_carrier = carrier;}

    /**
     * Sets ASIC number
     * @param asic number
     */
    void setASICNumber(unsigned short asic) {m_asic = asic;}

    /**
     * Sets ASIC channel number
     * @param channel number
     */
    void setASICChannel(unsigned short channel) {m_channel = channel;}

    /**
     * Sets storage window number
     * @param window number
     */
    void setASICWindow(unsigned short window) {m_window = window;}

    /**
     * Sets fine timing for 50% CFD (within two samples)
     * @param tfine fine timing
     */
    void setTFine(unsigned short tfine) {m_TFine = tfine;}

    /**
     * Sets ?
     * @param coffset ?
     */
    void setCOffset(unsigned short coffset) {m_COffset = coffset;}

    /**
     * Sets sample number just before 50% CFD crossing at leading edge
     * @param sample number
     */
    void setSample(unsigned short sample) {m_sample = sample;}

    /**
     * Sets peak position relative to m_sample
     * @param dsample sample difference
     */
    void setDeltaSamplePeak(unsigned short dsample) {m_dSamplePeak = dsample;}

    /**
     * Sets falling edge sample number just before 50% CFD crossing relative to m_sample
     * @param dsample sample difference
     */
    void setDeltaSampleFall(unsigned short dsample) {m_dSampleFall = dsample;}

    /**
     * Sets ADC value at m_sample
     * @param adc value
     */
    void setValueSample0(short adc) {m_VSample0 = adc;}

    /**
     * Sets ADC value at m_sample + 1
     * @param adc value
     */
    void setValueSample1(short adc) {m_VSample1 = adc;}

    /**
     * Sets ADC value at m_sample + m_dSamplePeak (e.g. pulse height)
     * @param adc value
     */
    void setValuePeak(short adc) {m_VPeak = adc;}

    /**
     * Sets ADC value at m_sample + m_dSampleFall
     * @param adc value
     */
    void setValueFall0(short adc) {m_VFall0 = adc;}

    /**
     * Sets ADC value at m_sample + m_dSampleFall + 1
     * @param adc value
     */
    void setValueFall1(short adc) {m_VFall1 = adc;}

    /**
     * Sets difference: m_VFall0 - m_VFall1
     * @param adc value
     */
    void setDeltaValueFall(short adc) {m_dVFall = adc;}

    /**
     * Sets integral of a pulse (e.g. a value proportional to charge)
     * @param integral
     */
    void setIntegral(short integral) {m_integral = integral;}

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const {return m_scrodID;}

    /**
     * Returns carrier board number
     * @return carrier board number
     */
    unsigned getCarrierNumber() const {return m_carrier;}

    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned getASICNumber() const {return m_asic;}

    /**
     * Returns ASIC channel number
     * @return ASIC channel number
     */
    unsigned getASICChannel() const {return m_channel;}

    /**
     * Returns ASIC storage window number
     * @return storage window number
     */
    unsigned getASICWindow() const {return m_window;}

    /**
     * Returns fine timing for 50% CFD (within two samples)
     * @return fine timing
     */
    unsigned getTFine() const {return m_TFine;}

    /**
     * Returns offset ?
     * @return offset
     */
    unsigned getCOffset() const {return m_COffset;}

    /**
     * Returns sample number at leading edge just before 50% CFD crossing
     * @return sample number
     */
    unsigned getSample() const {return m_sample;}

    /**
     * Returns peak position relative to m_sample
     * @return sample number difference
     */
    unsigned getDeltaSamplePeak() const {return m_dSamplePeak;}

    /**
     * Returns peak position
     * @return sample number
     */
    unsigned getSamplePeak() const {return m_sample + m_dSamplePeak;}

    /**
     * Returns sample number at falling edge just before 50% CFD relative to m_sample
     * @return sample number difference
     */
    unsigned getDeltaSampleFall() const {return m_dSampleFall;}

    /**
     * Returns sample number at falling edge just before 50% CFD crossing
     * @return sample number
     */
    unsigned getSampleFall() const {return m_sample + m_dSampleFall;}

    /**
     * Returns ADC value at leading edge (at m_sample)
     * @return ADC value
     */
    int getValueSample0() const {return m_VSample0;}

    /**
     * Returns ADC value at leading edge (at m_sample + 1)
     * @return ADC value
     */
    int getValueSample1() const {return m_VSample1;}

    /**
     * Returns ADC value at peak (e.g. pulse height)
     * @return ADC value
     */
    int getValuePeak() const {return m_VPeak;}

    /**
     * Returns ADC value at falling edge (at m_sample + m_dSampleFall)
     * @return ADC value
     */
    int getValueFall0() const {return m_VFall0;}

    /**
     * Returns ADC value at falling edge (at m_sample + m_dSampleFall + 1)
     * @return ADC value
     */
    int getValueFall1() const {return m_VFall1;}

    /**
     * Returns ADC value difference at falling edge (?)
     * @return ADC difference
     */
    int getDeltaValueFall() const {return m_dVFall;}

    /**
     * Returns integral of a pulse (e.g. a value proportional to charge)
     * @return integral
     */
    int getIntegral() const {return m_integral;}

    /**
     * Returns leading edge slope
     * @return slope [ADC counts per sample]
     */
    double getLeadingSlope() const {return getValueSample1() - getValueSample0();}

    /**
     * Returns falling edge slope
     * @return slope [ADC counts per sample]
     */
    double getFallingSlope() const {return getValueFall1() - getValueFall0();}

    /**
     * Returns leading edge CFD time
     * @return time [samples] (not in [ns], conversion factor is ~0.368 ns / sample)
     */
    double getCFDLeadingTime() const
    {
      return timeCFDCrossing(getSample(), getValueSample0(), getLeadingSlope());
    }

    /**
     * Returns falling edge CFD time
     * @return time [samples] (not in [ns], conversion factor is ~0.368 ns / sample)
     */
    double getCFDFallingTime() const
    {
      return timeCFDCrossing(getSampleFall(), getValueFall0(), getFallingSlope());
    }

    /**
     * Returns signal full width half maximum
     * @return FWHM [samples] (not in [ns], conversion factor is ~0.368 ns / sample)
     */
    double getFWHM() const {return getCFDFallingTime() - getCFDLeadingTime();}

    /**
     * Checks if leading edge is consistently defined
     * @return true if consistent
     */
    bool isLeadingEdgeValid() const {return checkEdge(m_VSample0, m_VSample1, m_VPeak);}

    /**
     * Checks if falling edge is consistently defined
     * @return true if consistent
     */
    bool isFallingEdgeValid() const {return checkEdge(m_VFall1, m_VFall0, m_VPeak);}

  private:

    /**
     * calculates time of 50% CFD crossing
     * @param sample sample number
     * @param value ADC value at sample number
     * @param slope slope of a line
     * @return time [samples]
     */
    double timeCFDCrossing(int sample, int value, double slope) const
    {
      if (slope == 0) return sample;
      return (int(m_VPeak) - 2 * value) / (2 * slope) + sample;
    }

    /**
     * Checks if values v0, v1 and vp are consistent
     */
    bool checkEdge(int v0, int v1, int vp) const
    {
      return (v1 > v0 and vp > 0) or (v1 < v0 and vp < 0);
    }

    unsigned short m_scrodID = 0;   /**< SCROD ID */
    unsigned short m_carrier = 0;   /**< carrier board number */
    unsigned short m_asic = 0;      /**< ASIC number */
    unsigned short m_channel = 0;   /**< ASIC channel number */
    unsigned short m_window = 0;    /**< storage window number */
    unsigned short m_TFine = 0;     /**< fine timing for 50% CFD (within two samples) */
    unsigned short m_COffset = 0;   /**< ? */
    unsigned short m_sample = 0;    /**< sample number just before 50% CFD crossing */
    unsigned short m_dSamplePeak = 0;  /**< peak position relative to m_sample */
    unsigned short m_dSampleFall = 0;  /**< same for falling edge, rel. to m_sample (?) */
    short m_VSample0 = 0;           /**< ADC value at m_sample */
    short m_VSample1 = 0;           /**< ADC value at m_sample + 1 */
    short m_VPeak = 0;              /**< ADC value at m_sample + m_dSamplePeak */
    short m_VFall0 = 0;             /**< ADC value at m_sample + m_dSampleFall */
    short m_VFall1 = 0;             /**< ADC value at m_sample + m_dSampleFall + 1 */
    short m_dVFall = 0;            /**< difference: m_VFall0 - m_VFall1 */
    short m_integral = 0;           /**< integral of a pulse (e.g. \propto charge) */

    ClassDef(TOPRawDigit, 1); /**< ClassDef */

  };


} // end namespace Belle2

