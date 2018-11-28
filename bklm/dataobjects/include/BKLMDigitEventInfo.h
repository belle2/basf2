/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store debugging informations from the unpacker (event based).
   */
  class BKLMDigitEventInfo : public RelationsObject {

  public:

    /**
     * Default constructor
     */
    BKLMDigitEventInfo():
      m_nOutOfRangeHits(0),
      m_nRPCHits(0),
      m_nSciHits(0),
      m_triggerCTime(0),
      m_triggerUTime(0),
      m_windowStart(0)
    {}

    /**
     * Increase by 1 the number of outOfRange-flagged hits in the event
     */
    void increaseOutOfRangeHits() { m_nOutOfRangeHits++; }

    /**
     * Increase by 'hits' the number of outOfRange-flagged hits in the event
     */
    void increaseOutOfRangeHits(int hits) { m_nOutOfRangeHits += hits; }

    /**
     * Increase by 1 the number of RPC hits in the event
     */
    void increaseRPCHits() { m_nRPCHits++; }

    /**
     * Increase by 'hits' the number of RPC hits in the event
     */
    void increaseRPCHits(int hits) { m_nRPCHits += hits; }

    /**
     * Increase by 1 the number of scintillator hits in the event
     */
    void increaseSciHits() { m_nSciHits++; }

    /**
     * Increase by 'hits' the number of scintillator hits in the event
     */
    void increaseSciHits(int hits) { m_nSciHits += hits; }

    /**
     * Set trigger CTime
     */
    void setTriggerCTime(unsigned int triggerCTime) { m_triggerCTime = triggerCTime; }

    /**
     * Set trigger UTime
     */
    void setTriggerUTime(unsigned int triggerUTime) { m_triggerUTime = triggerUTime; }

    /**
     * Set window start
     */
    void setWindowStart(unsigned int windowStart) { m_windowStart = windowStart; }

    /**
     * Set trigger CTime of previous event
     */
    void setPreviousEventTriggerCTime(unsigned int triggerCTimeOfPreviousEvent) { m_triggerCTimeOfPreviousEvent = triggerCTimeOfPreviousEvent; }

    /**
     * Returns the number of OutOfRange-flagged hits in the event
     * @return nOutOfRangeHits
     */
    int getOutOfRangeHits() const { return m_nOutOfRangeHits; }

    /**
     * Returns the number of RPC hits in the event
     * @return nRPCHits
     */
    int getRPCHits() const { return m_nRPCHits; }

    /**
     * Returns the number of scintillator hits in the event
     * @return nSciHits
     */
    int getSciHits() const { return m_nSciHits; }

    /**
     * Returns trigger CTime
     * @return trigger CTime
     */
    unsigned int getTriggerCTime() const { return m_triggerCTime; }

    /**
     * Returns trigger CTime as int
     * @return trigger CTime
     */
    int getIntTriggerCTime() const { return (int)m_triggerCTime; }

    /**
     * Returns trigger UTime
     * @return trigger UTime
     */
    unsigned int getTriggerUTime() const { return m_triggerUTime; }

    /**
     * Returns window start
     * @return window start
     */
    unsigned int getWindowStart() const { return m_windowStart; }

    /**
     * Returns trigger interval (triggerCTime - triggerCTimeOfPreviousEvent)
     * @return trigger interval
     */
    long int getTriggerInterval() const { return (long int)m_triggerCTime - (long int)m_triggerCTimeOfPreviousEvent; }

    /**
     * Returns trigger interval (triggerCTime - triggerCTimeOfPreviousEvent) in us
     * @return trigger interval in us
     */
    double getTriggerIntervalInUs() const { return ((long int)m_triggerCTime - (long int)m_triggerCTimeOfPreviousEvent) * 8.0 / 1000.0; }

  private:
    int m_nOutOfRangeHits; /**< outOfRange-flagged hits (skipped hits with layer > 14) */

    int m_nRPCHits; /**< RPC hits in the event */

    int m_nSciHits; /**< scintillator hits in the event */

    unsigned int m_triggerCTime; /**< trigger CTime */

    unsigned int m_triggerUTime; /**< trigger UTime */

    unsigned int m_windowStart; /**< window start */

    unsigned int m_triggerCTimeOfPreviousEvent; /**< trigger CTime of previous event */

    ClassDef(BKLMDigitEventInfo, 1); /**< ClassDef */

  };

} // end namespace Belle2
