/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oskar Hartbrich, Marko Staric                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store debugging information about the event headers in the  by the TOP production debugging raw data format one of these should be available per boardstack per event
   */
  class TOPProductionEventDebug : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPProductionEventDebug()
    {}

    /**
     * Full constructor
     * @param scrodID hardware SCROD ID
     */
    TOPProductionEventDebug(unsigned scrodID,
                            bool skipProcessingFlag,
                            unsigned short ctime,
                            unsigned short phase,
                            unsigned short asicMask,
                            unsigned short eventQueueDepth,
                            unsigned short eventNumberByte):
      m_scrodID(scrodID),
      m_skipProcessingFlag(skipProcessingFlag),
      m_ctime(ctime),
      m_phase(phase),
      m_asicMask(asicMask),
      m_eventQueueDepth(eventQueueDepth),
      m_eventNumberByte(eventNumberByte)
    {

    }


    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned short getScrodID() const { return m_scrodID; }

    /**
     * Returns skip processing bit
     * @return skip processing bit
     */
    bool getSkipProcessingFlag() const { return m_skipProcessingFlag; }


    /**
     * Returns event header ctime
     * @return event header ctime
     */
    unsigned short getCtime() const { return m_ctime; }

    /**
     * Returns event phase
     * @return event phase
     */
    unsigned short getPhase() const { return m_phase; }

    /**
     * Returns bit field of masked ASICs
     * @return asicMask
     */
    unsigned short getAsicMask() const { return m_asicMask; }

    /**
     * Returns trigger FIFO queue depth
     * @return eventQueueDepth
     */
    unsigned short getEventQueueDepth() const { return m_eventQueueDepth; }

    /**
     * Returns least significant byte of FE event number
     * @return least significant byte of FE event number
     */
    unsigned short getEventNumberByte() const { return m_eventNumberByte; }


  private:
    unsigned short m_scrodID = 0;       /**< hardware SCROD ID */
    bool m_skipProcessingFlag = false;      /**< skip processing bit, set if hit processing skipped due to queueDepth too high */
    unsigned short m_ctime = 0;       /**< event header ctime */
    unsigned short m_phase = 0;      /**< hardware phase */
    unsigned short m_asicMask = 0;       /**< bit field of masked ASICs (timeoutMask | setMask) */
    unsigned short m_eventQueueDepth = 0;       /**< trigger FIFO queue depth */
    unsigned short m_eventNumberByte = 0;       /**< frontend internal event number (8LSB) */


    ClassDef(TOPProductionEventDebug, 1); /**< ClassDef */

  };


} // end namespace Belle2
