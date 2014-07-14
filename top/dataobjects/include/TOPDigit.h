/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPDIGIT_H
#define TOPDIGIT_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /**
   * Class to store TOP digitized hits (output of TOPDigi)
   * relation to TOPSimHit
   * filled in top/modules/TOPDigitizer/src/TOPDigiModule.cc
   */

  class TOPDigit : public RelationsObject {
  public:

    /**
     * hit quality enumerators
     */
    enum EHitQuality {c_Junk, c_Good, c_ChargeShare, c_CrossTalk};

    /**
     * Default constructor
     */
    TOPDigit():
      m_barID(0),
      m_channelID(0),
      m_TDC(0),
      m_ADC(0),
      m_hardChannelID(0),
      m_quality(c_Junk)
    {}

    /** Almost full constructor
     * @param barID     bar ID
     * @param channelID channel ID
     * @param TDC       digitized detection time
     */
    TOPDigit(int barID, int channelID, int TDC):
      m_barID(barID),
      m_channelID(channelID),
      m_TDC(TDC),
      m_ADC(0),
      m_hardChannelID(0),
      m_quality(c_Good)
    {}

    /** Full constructor
     * @param barID     bar ID
     * @param channelID software channel ID
     * @param TDC       digitized detection time
     * @param ADC       digitized pulse height or integrated charge
     * @param hardChID  hardware channel ID
     */
    TOPDigit(int barID, int channelID, int TDC, int ADC, unsigned hardChID):
      m_barID(barID),
      m_channelID(channelID),
      m_TDC(TDC),
      m_ADC(ADC),
      m_hardChannelID(hardChID),
      m_quality(c_Good)
    {}

    /** Set hit quality
     * @param quality hit quality
     */
    void setHitQuality(EHitQuality quality) {m_quality = quality;}

    /** Get hit quality
     * @return hit quality
     */
    EHitQuality getHitQuality() const {return m_quality; }

    /** Get bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /** Get channel ID
     * @return software channel ID
     */
    int getChannelID() const { return m_channelID; }

    /** Get digitized time
     * @return digitized time
     */
    int getTDC() const { return m_TDC; }

    /** Get digitized pulse height or integrated charge
     * @return digitized pulse height or integrated charge
     */
    int getADC() const { return m_ADC; }

    /** Get hardware channel ID
     * @return hardware channel ID
     */
    int getHardChannelID() const { return m_hardChannelID; }

  private:
    int m_barID;               /**< bar ID (1-based) */
    int m_channelID;           /**< software channel ID (1-based) */
    int m_TDC;                 /**< digitized time */
    int m_ADC;                 /**< digitized pulse height or charge (to be decided) */
    unsigned m_hardChannelID;  /**< hardware channel ID (0-based) */
    EHitQuality m_quality;     /**< hit quality */

    ClassDef(TOPDigit, 3); /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif
