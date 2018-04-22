/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store trigger time stamps
   */
  class TOPTriggerDigit : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPTriggerDigit()
    {}

    /**
     * Useful constructor
     * @param moduleID slot number (1-based)
     * @param channel hardware channel number within a slot (0-based)
     * @param scrodID scrod ID
     */
    TOPTriggerDigit(int moduleID, unsigned channel, unsigned scrodID):
      m_moduleID(moduleID), m_channel(channel), m_scrodID(scrodID)
    {}

    /**
     * Append time stamp
     * @param timeStamp time stamp in 339MHz clock ticks since the most recent
     * frame9 (formerly known as revo9) marker
     */
    void appendTimeStamp(short timeStamp) {m_timeStamps.push_back(timeStamp);}

    /**
     * Returns module ID
     * @return slot number
     */
    int getModuleID() const { return m_moduleID; }

    /**
     * Returns hardware channel number
     * @return channel number
     */
    unsigned getChannel() const { return m_channel; }

    /**
     * Returns ASIC channel number
     * @return ASIC channel number
     */
    unsigned getASICChannel() const {return m_channel & 0x07;}

    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned getASICNumber() const {return (m_channel >> 3) & 0x03;}

    /**
     * Returns carrier board number
     * @return carrier board number
     */
    unsigned getCarrierNumber() const {return (m_channel >> 5) & 0x03;}

    /**
     * Returns boardstack number
     * @return boardstack number
     */
    unsigned getBoardstackNumber() const {return (m_channel >> 7) & 0x03;}

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const { return m_scrodID; }

    /**
     * Returns time stamps (339MHz clock ticks since the most recent frame9 marker)
     * @return time stamps
     */
    const std::vector<short>& getTimeStamps() const {return m_timeStamps;}

  private:

    int m_moduleID = 0;                 /**< module ID */
    unsigned m_channel = 0;             /**< hardware channel number */
    unsigned m_scrodID = 0;             /**< SCROD ID */
    std::vector<short> m_timeStamps;    /**< time stamps */

    ClassDef(TOPTriggerDigit, 1); /**< ClassDef */

  };

}
