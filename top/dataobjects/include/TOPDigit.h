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
    enum EHitQuality {c_Junk, c_Good, c_ChargeShare, c_CrossTalk};

    /**
     * Default constructor
     */
    TOPDigit()
    {}

    /**
     * Almost full constructor
     * @param moduleID     module ID (1-based)
     * @param pixelID   pixel ID (1-based)
     * @param TDC       digitized detection time
     */
    TOPDigit(int moduleID, int pixelID, int TDC):
      m_moduleID(moduleID),
      m_pixelID(pixelID),
      m_TDC(TDC),
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
     * Sets detection time
     * @param time time in [ns]
     */
    void setTime(double time) {m_time = time;}

    /**
     * Sets time uncertainty
     * @param timeError uncertainty (r.m.s.) in [ns]
     */
    void setTimeError(double timeError) {m_timeError = timeError;}

    /**
     * Sets digitized detection time
     * @param TDC digitized time
     */
    void setTDC(int TDC) {m_TDC = TDC;}

    /**
     * Sets pulse height
     * @param ADC pulse height [ADC counts]
     */
    void setADC(int ADC) {m_ADC = ADC;}

    /**
     * Sets pulse integral
     * @param integral pulse integral [ADC counts]
     */
    void setIntegral(int integral) {m_integral = integral;}

    /**
     * Sets pulse width
     * @param width pulse width (FWHM) in [ns]
     */
    void setPulseWidth(double width) {m_pulseWidth = width;}

    /**
     * Sets hardware channel number (0-based)
     * @param channel hardware channel number
     */
    void setChannel(unsigned int channel) {m_channel = channel;}

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
     * Returns t0-subtracted and calibrated time (converted back from TDC counts)
     * @return time in [ns]
     */
    double getTime() const { return m_time; }

    /**
     * Returns time uncertainty
     * @return uncertainty (r.m.s.) in [ns]
     */
    double getTimeError() const {return m_timeError;}

    /**
     * Returns digitized time
     * @return digitized time
     */
    int getTDC() const { return m_TDC; }

    /**
     * Returns pulse height
     * @return pulse height [ADC counts]
     */
    int getADC() const { return m_ADC; }

    /**
     * Returns pulse integral
     * @return pulse integral [ADC counts]
     */
    int getIntegral() const {return m_integral;}

    /**
     * Returns pulse width
     * @return pulse width (FWHM) in [ns]
     */
    double getPulseWidth() const { return m_pulseWidth; }

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
    int m_TDC = 0;            /**< digitized time */
    int m_ADC = 0;            /**< pulse height [ADC counts] */
    int m_integral = 0;       /**< pulse integral [ADC counts] */
    unsigned m_channel = 0;   /**< hardware channel number (0-based) */
    EHitQuality m_quality = c_Junk;  /**< hit quality */
    float m_time = 0;         /**< calibrated time in [ns], t0-subtracted */
    float m_timeError = 0;    /**< time uncertainty (r.m.s) in [ns] */
    float m_pulseWidth = 0;   /**< pulse width (FWHM) in [ns] */
    unsigned short m_firstWindow = 0; /**< first ASIC window of the merged waveform */

    static float s_doubleHitResolution; /**< double hit resolving time in [ns] */
    static float s_pileupTime; /**< pile-up time in [ns] */

    ClassDef(TOPDigit, 12); /**< ClassDef */

  };


} // end namespace Belle2

