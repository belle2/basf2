/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/DigitBase.h>
#include <framework/logging/Logger.h>
#include <iostream>

namespace Belle2 {

  /**
   * Class to store TOP digitized hits (output of TOPDigitizer or raw data unpacker)
   * relations to TOPSimHits, MCParticles
   */

  class TOPDigit : public DigitBase {
  public:

    /**
     * hit quality enumerators
     */
    enum EHitQuality {
      c_Junk = 0,
      c_Good = 1,
      c_CrossTalk = 3,
      c_CalPulse = 4
    };

    /**
     * calibration status enumerators
     */
    enum EStatusBits {
      c_TimeBaseCalibrated =  1,
      c_ChannelT0Calibrated = 2,
      c_ModuleT0Calibrated =  4,
      c_CommonT0Calibrated =  8,
      c_FullyCalibrated = c_TimeBaseCalibrated | c_ChannelT0Calibrated | c_ModuleT0Calibrated | c_CommonT0Calibrated,
      c_OffsetSubtracted = 16,
      c_EventT0Subtracted = 32,
    };

    /**
     * charge sharing enumerators
     */
    enum EChargeShare {
      c_PrimaryChargeShare = 1, /**< the largest one among hits sharing the same charge */
      c_SecondaryChargeShare = 2  /**< others sharing the same charge */
    };

    /**
     * Default constructor
     */
    TOPDigit()
    {}

    /**
     * Usefull constructor
     * @param moduleID  module ID (1-based)
     * @param pixelID   pixel ID (1-based)
     * @param rawTime   raw time expressed in samples (TDC bins)
     */
    TOPDigit(int moduleID, int pixelID, double rawTime):
      m_moduleID(moduleID),
      m_pixelID(pixelID),
      m_rawTime(rawTime),
      m_quality(c_Good)
    {}

    /**
     * Sets double hit resolution
     * @param time double hit resolving time in [ns]
     */
    static void setDoubleHitResolution(double time) {s_doubleHitResolution = time;}

    /**
     * Sets pile-up time
     * @param time pile-up time in [ns]
     */
    static void setPileupTime(double time) {s_pileupTime = time;}

    /**
     * Sets hardware channel number (0-based)
     * @param channel hardware channel number
     */
    void setChannel(unsigned int channel) {m_channel = channel;}

    /**
     * Sets raw detection time
     * @param rawTime raw time expressed in samples (TDC bins)
     */
    void setRawTime(double rawTime) {m_rawTime = rawTime;}

    /**
     * Sets calibrated detection time
     * @param time time in [ns]
     */
    void setTime(double time) {m_time = time;}

    /**
     * Sets calibrated time uncertainty
     * @param timeError uncertainty (r.m.s.) in [ns]
     */
    void setTimeError(double timeError) {m_timeError = timeError;}

    /**
     * Sets pulse height
     * @param pulseHeight pulse height [ADC counts]
     */
    void setPulseHeight(int pulseHeight) {m_pulseHeight = pulseHeight;}

    /**
     * Sets pulse width
     * @param width pulse width (FWHM) in [ns]
     */
    void setPulseWidth(double width) {m_pulseWidth = width;}

    /**
     * Sets pulse integral
     * @param integral pulse integral [ADC counts]
     */
    void setIntegral(int integral) {m_integral = integral;}

    /**
     * Sets first ASIC window number of the merged waveform this hit is taken from
     * @param window ASIC window number
     */
    void setFirstWindow(unsigned window) { m_firstWindow = window;}

    /**
     * Sets hit quality flag
     * @param quality hit quality
     */
    void setHitQuality(EHitQuality quality) {m_quality = quality;}

    /**
     * Sets calibration status (overwrites previously set bits)
     */
    void setStatus(unsigned short status) { m_status = status; }

    /**
     * Add calibration status
     */
    void addStatus(unsigned short bitmask) { m_status |= bitmask; }

    /**
     * Remove calibration status
     */
    void removeStatus(unsigned short bitmask) { m_status &= (~bitmask); }

    /**
     * Sets primary charge share flag
     */
    void setPrimaryChargeShare() {m_chargeShare = c_PrimaryChargeShare;}

    /**
     * Sets secondary charge share flag
     */
    void setSecondaryChargeShare() {m_chargeShare = c_SecondaryChargeShare;}

    /**
     * Remove charge share flag
     */
    void resetChargeShare() {m_chargeShare = 0;}

    /**
     * Subtract start time from m_time
     * @param t0 start time in [ns]
     */
    void subtractT0(double t0) { m_time -= t0;}

    /**
     * Returns hit quality
     * @return hit quality
     */
    EHitQuality getHitQuality() const {return m_quality; }

    /**
     * Returns calibration status
     * @return calibration status
     */
    bool hasStatus(unsigned short bitmask) const
    {
      return (m_status & bitmask) == bitmask;
    }

    /**
     * Returns calibration status
     * @return true, if fully calibrated
     */
    bool isCalibrated() const {return hasStatus(c_FullyCalibrated);}

    /**
     * Returns calibration status
     * @return true, if time base calibrated
     */
    bool isTimeBaseCalibrated() const {return hasStatus(c_TimeBaseCalibrated);}

    /**
     * Returns calibration status
     * @return true, if channel T0 calibrated
     */
    bool isChannelT0Calibrated() const {return hasStatus(c_ChannelT0Calibrated);}

    /**
     * Returns calibration status
     * @return true, if module T0 calibrated
     */
    bool isModuleT0Calibrated() const {return hasStatus(c_ModuleT0Calibrated);}

    /**
     * Returns calibration status
     * @return true, if common T0 calibrated
     */
    bool isCommonT0Calibrated() const {return hasStatus(c_CommonT0Calibrated);}

    /**
     * Returns charge share status
     * @return true, if digit is sharing charge with some other digits
     */
    bool isChargeShare() const {return m_chargeShare != 0;}

    /**
     * Returns charge share status
     * @return true, if digit is the primary one among those sharing the same charge
     */
    bool isPrimaryChargeShare() const {return m_chargeShare == c_PrimaryChargeShare;}

    /**
     * Returns charge share status
     * @return true, if digit is not the primary one among those sharing the same charge
     */
    bool isSecondaryChargeShare() const {return m_chargeShare == c_SecondaryChargeShare;}

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const { return m_moduleID; }

    /**
     * Returns pixel ID (1-based)
     * @return software channel ID
     */
    int getPixelID() const { return m_pixelID; }

    /**
     * Returns pixel row number (1-based)
     * @return pixel row number
     */
    int getPixelRow() const { return (m_pixelID - 1) / 64 + 1;}

    /**
     * Returns pixel column number (1-based)
     * @return pixel column number
     */
    int getPixelCol() const { return (m_pixelID - 1) % 64 + 1;}

    /**
     * Returns PMT row number (1-based)
     * @return PMT row number
     */
    int getPMTRow() const {return (getPixelRow() - 1) / 4 + 1;}

    /**
     * Returns PMT column number (1-based)
     * @return PMT column number
     */
    int getPMTCol() const {return (getPixelCol() - 1) / 4 + 1;}

    /**
     * Returns PMT number (1-based)
     * @return PMT number
     */
    int getPMTNumber() const {return getPMTCol() + (getPMTRow() - 1) * 16;}

    /**
     * Returns PMT pixel row number (1-based)
     * @return PMT pixel row number
     */
    int getPMTPixelRow() const {return (getPixelRow() - 1) % 4 + 1;}

    /**
     * Returns PMT pixel column number (1-based)
     * @return PMT pixel column number
     */
    int getPMTPixelCol() const {return (getPixelCol() - 1) % 4 + 1;}

    /**
     * Returns PMT pixel number (1-based)
     * @return PMT pixel
     */
    int getPMTPixel() const {return getPMTPixelCol() + (getPMTPixelRow() - 1) * 4;}

    /**
     * Returns hardware channel number
     * @return hardware channel number
     */
    unsigned int getChannel() const { return m_channel; }

    /**
     * Returns ASIC channel number
     * @return ASIC channel number
     */
    unsigned int getASICChannel() const {return m_channel & 0x07;}

    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned int getASICNumber() const {return (m_channel >> 3) & 0x03;}

    /**
     * Returns carrier board number
     * @return carrier board number
     */
    unsigned int getCarrierNumber() const {return (m_channel >> 5) & 0x03;}

    /**
     * Returns boardstack number
     * @return boardstack number
     */
    unsigned int getBoardstackNumber() const {return (m_channel >> 7) & 0x03;}

    /**
     * Returns raw detection time
     * @return time expressed in samples (TDC bins)
     */
    double getRawTime() const { return m_rawTime; }

    /**
     * Returns sample number modulo 256
     * @return sample number modulo 256
     */
    int getModulo256Sample() const;

    /**
     * Returns t0-subtracted and calibrated time
     * @return time in [ns]
     */
    double getTime() const { return m_time; }

    /**
     * Returns calibrated time uncertainty
     * @return uncertainty (r.m.s.) in [ns]
     */
    double getTimeError() const {return m_timeError;}

    /**
     * Returns pulse height
     * @return pulse height [ADC counts]
     */
    int getPulseHeight() const { return m_pulseHeight; }

    /**
     * Returns pulse width
     * @return pulse width (FWHM) in [ns]
     */
    double getPulseWidth() const { return m_pulseWidth; }

    /**
     * Returns pulse integral
     * @return pulse integral [ADC counts]
     */
    int getIntegral() const {return m_integral;}

    /**
     * Returns first ASIC window number of the merged waveform this hit is taken from
     * @return window number
     */
    unsigned getFirstWindow() const { return m_firstWindow;}

    /**
     * Implementation of the base class function.
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * @return unique channel ID, composed of pixel ID (1-512) and module ID (1-16)
     */
    unsigned int getUniqueChannelID() const {return m_pixelID + (m_moduleID << 16);}

    /**
     * Implementation of the base class function.
     * Pile-up method.
     * @param bg BG digit
     * @return append status
     */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg);


  private:
    int m_moduleID = 0;       /**< module ID (1-based) */
    int m_pixelID = 0;        /**< software channel ID (1-based) */
    unsigned m_channel = 0;   /**< hardware channel number (0-based) */
    float m_rawTime = 0;      /**< raw time expressed in samples (TDC bins) */
    float m_time = 0;         /**< calibrated time in [ns], t0-subtracted */
    float m_timeError = 0;    /**< time uncertainty (r.m.s) in [ns] */
    int m_pulseHeight = 0;    /**< pulse height [ADC counts] */
    float m_pulseWidth = 0;   /**< pulse width (FWHM) in [ns] */
    int m_integral = 0;       /**< pulse integral [ADC counts] */
    unsigned short m_firstWindow = 0; /**< first ASIC window of the merged waveform */
    EHitQuality m_quality = c_Junk;  /**< hit quality */
    unsigned short m_status = 0; /**< calibration status bits */
    unsigned short m_chargeShare = 0; /**< charge sharing flags */

    static float s_doubleHitResolution; /**< double hit resolving time in [ns] */
    static float s_pileupTime; /**< pile-up time in [ns] */

    ClassDef(TOPDigit, 15); /**< ClassDef */

  };


} // end namespace Belle2

