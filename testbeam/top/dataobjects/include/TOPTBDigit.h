/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPTBDIGIT_H
#define TOPTBDIGIT_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store beam test data for any counter other than TOP
   */
  class TOPTBDigit : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPTBDigit():
      m_counterID(0),
      m_type(0),
      m_channelID(0),
      m_hardChannelID(0),
      m_TDC(0),
      m_ADC(0) {
    }

    /** Full constructor
     * @param counterID counter ID
     * @param type counter type
     * @param channelID software channel ID
     * @param TDC       digitized time
     * @param ADC       digitized pulse hight or charge
     * @param hardchID  hardware channel ID
     */
    TOPTBDigit(int counterID, int type, int channelID, int TDC, int ADC,
               unsigned hardchID = 0):
      m_counterID(counterID),
      m_type(type),
      m_channelID(channelID),
      m_hardChannelID(hardchID),
      m_TDC(TDC),
      m_ADC(ADC) {
    }

    /** Get counter ID
     * @return counter ID
     */
    int getCounterID() const { return m_counterID; }

    /** Get counter type
    * @return counter type
    */
    int getType() const { return m_type;}

    /** Get software channel ID
     * @return channel ID
     */
    int getChannelID() const { return m_channelID; }

    /** Get hardware channel ID
     * @return channel ID
     */
    int getHardChannelID() const { return m_hardChannelID; }

    /** Get digitized time
     * @return TDC value
     */
    int getTDC() const { return m_TDC; }

    /** Get digitized pulse hight or charge
     * @return ADC value
     */
    int getADC() const { return m_ADC; }

  private:
    int m_counterID;          /**< counter ID */
    int m_type;               /**< counter type */
    int m_channelID;          /**< software channel ID */
    unsigned m_hardChannelID; /**< hardware channel ID */
    int m_TDC;                /**< digitized time */
    int m_ADC;                /**< digitized pulse hight or charge */

    ClassDef(TOPTBDigit, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif
