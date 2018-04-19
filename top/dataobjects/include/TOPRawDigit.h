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
   * It provides also calculation of 50% CFD leading and falling edge times and errors
   */

  class TOPRawDigit : public RelationsObject {
  public:

    /**
     * Enum for error flags; bits set if corresponding data not consistent
     */
    enum ErrorFlags {
      c_HeadMagic = 0x0001,  /**< if magic number not 0xA */
      c_TailMagic = 0x0002,  /**< if magic bits not '101' = 0x5 */
      c_HitMagic = 0x0004,   /**< if magic number not 0xB */
      c_HitChecksum = 0x0008 /**< if sum of 16-bit words not zero */
    };

    /**
     * Enum for data types
     * needed to steer time conversion in TOPRawDigitConverter
     */
    enum EDataTypes {
      c_Undefined = 0,         /**< undefined */
      c_MC = 1,                /**< from MC digitization */
      c_Interim = 2,           /**< from interim feature extraction */
      c_Production = 3,        /**< from the future production format */
      c_ProductionDebug = 4,   /**< from production debugging format */
      c_Other = 99             /**< other */
    };

    /**
     * Various constants
     */
    enum {
      c_WindowSize = 64 /**< number of samples per window */
    };

    /**
     * Default constructor
     */
    TOPRawDigit()
    {}

    /**
     * Usefull constructor
     * @param scrodID SCROD ID
     */
    explicit TOPRawDigit(unsigned short scrodID, EDataTypes dataType):
      m_scrodID(scrodID), m_dataType(dataType)
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
     * Sets first storage window number (logical window number)
     * @param window number
     */
    void setASICWindow(unsigned short window) {m_window = window;}

    /**
     * Sets storage windows of waveform segments
     * @param windows window numbers
     */
    void setStorageWindows(const std::vector<unsigned short>& windows)
    {
      m_windows = windows;
    }

    /**
     * Sets current (reference) window number
     * @param window number
     */
    void setLastWriteAddr(unsigned short window) {m_lastWriteAddr = window;}

    /**
     * Sets fine timing for 50% CFD at rising edge (within two samples)
     * @param tfine fine timing
     */
    void setTFine(unsigned short tfine) {m_TFine = tfine;}

    /**
     * Sets sample number just before 50% CFD crossing at leading edge
     * @param sample number
     */
    void setSampleRise(unsigned short sample) {m_sampleRise = sample;}

    /**
     * Sets peak position relative to m_sampleRise
     * @param dsample sample difference
     */
    void setDeltaSamplePeak(unsigned short dsample) {m_dSamplePeak = dsample;}

    /**
     * Sets falling edge sample number just before 50% CFD crossing
     * relative to m_sampleRise
     * @param dsample sample difference
     */
    void setDeltaSampleFall(unsigned short dsample) {m_dSampleFall = dsample;}

    /**
     * Sets ADC value at m_sampleRise
     * @param adc value
     */
    void setValueRise0(short adc) {m_VRise0 = adc;}

    /**
     * Sets ADC value at m_sampleRise + 1
     * @param adc value
     */
    void setValueRise1(short adc) {m_VRise1 = adc;}

    /**
     * Sets ADC value at m_sampleRise + m_dSamplePeak (e.g. pulse height)
     * @param adc value
     */
    void setValuePeak(short adc) {m_VPeak = adc;}

    /**
     * Sets ADC value at m_sampleRise + m_dSampleFall
     * @param adc value
     */
    void setValueFall0(short adc) {m_VFall0 = adc;}

    /**
     * Sets ADC value at m_sampleRise + m_dSampleFall + 1
     * @param adc value
     */
    void setValueFall1(short adc) {m_VFall1 = adc;}

    /**
     * Sets integral of a pulse (e.g. a value proportional to charge)
     * @param integral
     */
    void setIntegral(int integral) {m_integral = integral;}

    /**
     * Sets number of global clock tics since last revo9 flag (production firmware only)
     * @param revo9counter
     */
    void setRevo9Counter(unsigned short revo9Counter) {m_revo9Counter = revo9Counter;}

    /**
     * Sets beam orbit synchronisation phase (production firmware only)
     * 9-state count: valid values are 0 - 8
     * @param beam orbit sunchronisation phase
     */
    void setPhase(unsigned short phase) {m_phase = phase;}

    /**
     * Sets error flags
     * @param flags error flags
     */
    void setErrorFlags(unsigned short flags) {m_errorFlags = flags;}

    /**
     * Sets offline flag: telling that this digit was extracted offline in basf2
     */
    void setOfflineFlag() {m_offline = true;}

    /**
     * Returns data type
     * @return data type
     */
    EDataTypes getDataType() const {return m_dataType;}

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
     * Returns channel number within SCROD (in the range 0 - 127)
     * @return SCROD channel number
     */
    unsigned getScrodChannel() const {return m_channel + m_asic * 8 + m_carrier * 32;}

    /**
     * Returns ASIC storage window number
     * @return window number
     */
    unsigned getASICWindow() const {return m_window;}

    /**
     * Returns storage window numbers of waveform segments (not always available!)
     * @return window numbers
     */
    const std::vector<unsigned short>& getStorageWindows() const {return m_windows;}

    /**
     * Returns current (reference) ASIC window number
     * @return window number
     */
    unsigned getLastWriteAddr() const {return m_lastWriteAddr;}

    /**
     * Returns fine timing for 50% CFD (within two samples)
     * @return fine timing
     */
    unsigned getTFine() const {return m_TFine;}

    /**
     * Returns sample number at leading edge just before 50% CFD crossing
     * @return sample number
     */
    unsigned getSampleRise() const {return m_sampleRise;}

    /**
     * Returns peak position relative to m_sampleRise
     * @return sample number difference
     */
    unsigned getDeltaSamplePeak() const {return m_dSamplePeak;}

    /**
     * Returns peak position
     * @return sample number
     */
    unsigned getSamplePeak() const {return m_sampleRise + m_dSamplePeak;}

    /**
     * Returns sample number at falling edge just before 50% CFD relative to m_sampleRise
     * @return sample number difference
     */
    unsigned getDeltaSampleFall() const {return m_dSampleFall;}

    /**
     * Returns sample number at falling edge just before 50% CFD crossing
     * @return sample number
     */
    unsigned getSampleFall() const {return m_sampleRise + m_dSampleFall;}

    /**
     * Returns ADC value at leading edge (at m_sampleRise)
     * @return ADC value
     */
    int getValueRise0() const {return m_VRise0;}

    /**
     * Returns ADC value at leading edge (at m_sampleRise + 1)
     * @return ADC value
     */
    int getValueRise1() const {return m_VRise1;}

    /**
     * Returns ADC value at peak (e.g. pulse height)
     * @return ADC value
     */
    int getValuePeak() const {return m_VPeak;}

    /**
     * Returns ADC value at falling edge (at m_sampleRise + m_dSampleFall)
     * @return ADC value
     */
    int getValueFall0() const {return m_VFall0;}

    /**
     * Returns ADC value at falling edge (at m_sampleRise + m_dSampleFall + 1)
     * @return ADC value
     */
    int getValueFall1() const {return m_VFall1;}

    /**
     * Returns integral of a pulse (e.g. a value proportional to charge)
     * @return integral
     */
    int getIntegral() const {return m_integral;}

    /**
     * Returns 127 MHz clock ticks since last revo9 marker
     * @return revo9counter
     */
    unsigned short getRevo9Counter() const {return m_revo9Counter;}

    /**
     * Returns beam orbit synchronisation phase (9-state count: valid values are 0 - 8)
     * @return phase
     */
    unsigned short getPhase() const {return m_phase;}

    /**
     * Returns error flags
     * @return flags
     */
    unsigned short getErrorFlags() const {return m_errorFlags;}

    /**
     * Returns leading edge slope
     * @return slope [ADC counts per sample]
     */
    double getLeadingSlope() const {return getValueRise1() - getValueRise0();}

    /**
     * Returns falling edge slope
     * @return slope [ADC counts per sample]
     */
    double getFallingSlope() const {return getValueFall1() - getValueFall0();}

    /**
     * Returns leading edge CFD time
     * @return time [samples] (not in [ns]!)
     */
    double getCFDLeadingTime() const
    {
      return timeCFDCrossing(getSampleRise(), getValueRise0(), getLeadingSlope());
    }

    /**
     * Returns falling edge CFD time
     * @return time [samples] (not in [ns]!)
     */
    double getCFDFallingTime() const
    {
      return timeCFDCrossing(getSampleFall(), getValueFall0(), getFallingSlope());
    }

    /**
     * Returns leading edge CFD time uncertainty (assuming uncorrelated noise)
     * @param rmsNoise r.m.s of the pedestal fluctuations [ADC counts]
     * @return time uncertainty [samples] (not in [ns]!)
     */
    double getCFDLeadingTimeError(double rmsNoise) const
    {
      return rmsNoise * timeErrorCoefficient(m_VRise0, m_VRise1);
    }

    /**
     * Returns falling edge CFD time uncertainty (assuming uncorrelated noise)
     * @param rmsNoise r.m.s of the pedestal fluctuations [ADC counts]
     * @return time uncertainty [samples] (not in [ns]!)
     */
    double getCFDFallingTimeError(double rmsNoise) const
    {
      return rmsNoise * timeErrorCoefficient(m_VFall0, m_VFall1);
    }

    /**
     * Returns signal full width half maximum
     * @return FWHM [samples] (not in [ns]!)
     */
    double getFWHM() const {return getCFDFallingTime() - getCFDLeadingTime();}

    /**
     * Checks if leading edge is consistently defined
     * @return true if consistent
     */
    bool isLeadingEdgeValid() const {return checkEdge(m_VRise0, m_VRise1, m_VPeak);}

    /**
     * Checks if falling edge is consistently defined
     * @return true if consistent
     */
    bool isFallingEdgeValid() const {return checkEdge(m_VFall1, m_VFall0, m_VPeak);}

    /**
     * Checks if feature extraction points make sense
     * @return true if consistent
     */
    bool isFEValid() const
    {
      return isLeadingEdgeValid() and isFallingEdgeValid();
    }

    /**
     * Checks if feature extraction finds a pedestal jump
     * @return true if pedestal jump
     */
    bool isPedestalJump() const;

    /**
     * Checks if feature extraction points are at window discontinuity
     * (e.g. discontinuity happens between sampleRise and sampleFall+1)
     * NOTE: always false if m_windows is empty
     * @param storageDepth storage depth
     * @return true if window discontinuity is found between sampleRise and sampleFall+1
     */
    bool isAtWindowDiscontinuity(unsigned short storageDepth = 508) const;

    /**
     * Checks if storage windows come in the consecutive order before the last sample
     * (no gaps before the last sample)
     * Note: returns true if m_windows is empty
     * @param storageDepth storage depth
     * @return true, if no gaps before the last sample or m_windows is empty
     */
    bool areWindowsInOrder(unsigned short storageDepth = 508) const;

    /**
     * Corrects time after window discontinuity by adding missing samples
     * @param time leading or falling edge time [samples]
     * @param storageDepth storage depth
     * @return time corrected for missing samples if any, otherwise returns input value [samples]
     */
    double correctTime(double time, unsigned short storageDepth = 508) const;

    /**
     * Checks if the first window number is the same as the first one in m_windows
     * Note: returns true if m_windows is empty
     * @return true, if window numbers are the same or m_windows is empty
     */
    bool isWindowConsistent() const
    {
      if (m_windows.empty()) return true;
      return m_windows[0] == m_window;
    }

    /**
     * Returns offline flag
     * @return true, if digit was extracted from waveform offline (in basf2)
     */
    bool isMadeOffline() const {return m_offline;}

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
     * Calculate the coefficient of time error
     * @param y1 first value (e.g. VRise0 or VFall0)
     * @param y2 second value (e.g. VRise1 or VFall1)
     * @return coefficient of time error
     */
    double timeErrorCoefficient(double y1, double y2) const;

    /**
     * Checks if values v1, v2 and vp are consistent
     */
    bool checkEdge(int v1, int v2, int vp) const
    {
      return (vp > 0 and v1 < v2 and 2 * v1 <= vp and v2 <= vp)
             or (vp < 0 and v1 > v2 and 2 * v1 >= vp and v2 >= vp);
    }

    unsigned short m_scrodID = 0; /**< SCROD ID */
    unsigned short m_carrier = 0; /**< carrier board number */
    unsigned short m_asic = 0;    /**< ASIC number */
    unsigned short m_channel = 0; /**< ASIC channel number */
    unsigned short m_window = 0;  /**< first ASIC storage window number */
    unsigned short m_TFine = 0; /**< fine timing for 50% CFD (within two samples) */
    unsigned short m_sampleRise = 0;  /**< sample number just before 50% CFD crossing */
    unsigned short m_dSamplePeak = 0; /**< peak position relative to m_sampleRise */
    unsigned short m_dSampleFall = 0; /**< same for falling edge, rel. to m_sampleRise */
    short m_VRise0 = 0;       /**< ADC value at m_sampleRise */
    short m_VRise1 = 0;       /**< ADC value at m_sampleRise + 1 */
    short m_VPeak = 0;        /**< ADC value at m_sampleRise + m_dSamplePeak */
    short m_VFall0 = 0;       /**< ADC value at m_sampleRise + m_dSampleFall */
    short m_VFall1 = 0;       /**< ADC value at m_sampleRise + m_dSampleFall + 1 */
    int m_integral = 0;     /**< integral of a pulse (e.g. \propto charge) */
    unsigned short m_revo9Counter = 0; /**< number of clock ticks since last revo9 flag */
    unsigned short m_phase = 0; /**< carrier phase */

    unsigned short m_errorFlags = 0; /**< feature extraction error flags (see enum) */
    unsigned short m_lastWriteAddr = 0; /**< current (reference) window number */
    std::vector<unsigned short> m_windows; /**< storage windows of waveform segments */
    bool m_offline = false; /**< feature extraction flag: by firmware or software */
    EDataTypes m_dataType = c_Undefined; /**< data type */

    ClassDef(TOPRawDigit, 5); /**< ClassDef */

  };


} // end namespace Belle2

