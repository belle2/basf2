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
     * @param barID     bar ID (1-based)
     * @param pixelID   pixel ID (1-based)
     * @param TDC       digitized detection time
     */
    TOPDigit(int barID, int pixelID, int TDC):
      m_barID(barID),
      m_pixelID(pixelID),
      m_TDC(TDC),
      m_ADC(0),
      m_pulseWidth(0),
      m_channelID(0),
      m_quality(c_Good),
      m_time(0)
    {}

    /**
     * Set time in [ns]
     * @param time time in [ns]
     */
    void setTime(double time) {m_time = time;}

    /**
     * Set digitized detection time
     * @param TDC digitized time
     */
    void setTDC(int TDC) {m_TDC = TDC;}

    /**
     * Set digitized pulse height or integrated charge
     * @param ADC pulse heigth or integrated charge
     */
    void setADC(int ADC) {m_ADC = ADC;}

    /**
     * Set digitized pulse width
     * @param width pulse width
     */
    void setPulseWidth(int width) {m_pulseWidth = width;}

    /**
     * Set hardware channel ID (0-based)
     * @param channel hardware channel ID
     */
    void setChannelID(unsigned int channel) {m_channelID = channel;}

    /**
     * Set hardware channel ID (0-based)
     * @param channel hardware channel ID
     */
    void setHardwareChannelID(unsigned int channel)  __attribute__((deprecated("Please use setChannelID()")))
    {m_channelID = channel;}

    /**
     * Set hit quality
     * @param quality hit quality
     */
    void setHitQuality(EHitQuality quality) {m_quality = quality;}

    /**
     * Set double hit resolution
     * @param time double hit resolving time in [ns]
     */
    static void setDoubleHitResolution(double time) {s_doubleHitResolution = time;}

    /**
     * Set pile-up time
     * @param time pile-up time in [ns]
     */
    static void setPileupTime(double time) {s_pileupTime = time;}

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
    int getBarID() const { return m_barID; }

    /**
     * Returns pixel ID
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
     * Returns pixel ID
     * @return software channel ID
     */
    int getChannelID() const  __attribute__((deprecated("Please use getPixelID()")))
    { return m_pixelID; }

    /**
     * Returns t0-subtracted and calibrated time in [ns] (converted back from TDC counts)
     * @return time
     */
    double getTime() const { return m_time; }

    /**
     * Returns digitized time
     * @return digitized time
     */
    int getTDC() const { return m_TDC; }

    /**
     * Returns digitized pulse height or integrated charge
     * @return digitized pulse height or integrated charge
     */
    int getADC() const { return m_ADC; }

    /**
     * Returns digitized pulse width
     * @return digitized pulse width
     */
    int getPulseWidth() const { return m_pulseWidth; }

    /**
     * Returns hardware channel ID
     * @return hardware channel ID
     */
    unsigned int getHardwareChannelID() const { return m_channelID; }

    /**
     * Returns ASIC channel number
     * @return ASIC channel number
     */
    unsigned int getASICChannel() const {return m_channelID & 0x07;}


    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned int getASICNumber() const {return (m_channelID >> 3) & 0x03;}

    /**
     * Returns carrier board number
     * @return carrier board number
     */
    unsigned int getCarrierNumber() const {return (m_channelID >> 5) & 0x03;}

    /**
     * Returns boardstack number
     * @return boardstack number
     */
    unsigned int getBoardstackNumber() const {return (m_channelID >> 7) & 0x03;}

    /**
     * Implementation of the base class function.
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * @return unique channel ID, composed of channel ID (1-512) and bar ID (1-16)
     */
    unsigned int getUniqueChannelID() const {return m_pixelID + (m_barID << 16);}

    /**
     * Implementation of the base class function.
     * Pile-up method.
     * @param bg BG digit
     * @return append status
     */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg);


  private:
    int m_barID = 0;         /**< module ID (1-based) */
    int m_pixelID = 0;       /**< software channel ID (1-based) */
    int m_TDC = 0;           /**< digitized time */
    int m_ADC = 0;           /**< digitized pulse height or charge (to be decided) */
    int m_pulseWidth = 0;    /**< digitized pulse width */
    unsigned m_channelID = 0;   /**< hardware channel ID (0-based) */
    EHitQuality m_quality = c_Junk;  /**< hit quality */
    float m_time = 0;        /**< time in [ns], converted from TDC, t0-subtracted */

    static float s_doubleHitResolution; /**< double hit resolving time in [ns] */
    static float s_pileupTime; /**< pile-up time in [ns] */

    ClassDef(TOPDigit, 8); /**< ClassDef */

  };


} // end namespace Belle2

