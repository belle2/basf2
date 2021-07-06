/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>
#include <rawdata/dataobjects/RawKLM.h>

namespace Belle2 {

  /**
   * Class to store debugging informations from the unpacker (event based).
   */
  class KLMDigitEventInfo : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    KLMDigitEventInfo():
      m_TriggerCTime(0),
      m_triggerCTimeOfPreviousEvent(0),
      m_triggerUTime(0),
      m_windowStart(0),
      m_nRPCHits(0),
      m_nSciHits(0),
      m_nOutOfRangeHits(0),
      m_Revo9TriggerWord(0),
      m_UserWord(0)
    {
    };

    /**
     * Constructor.
     * @param[in] rawKLM RawKLM.
     * @param[in] entry  Entry (corresponds to the data form one copper).
     */
    KLMDigitEventInfo(RawKLM* rawKLM, int entry) :
      m_TriggerCTime(rawKLM->GetTTCtime(entry)),
      m_triggerCTimeOfPreviousEvent(0),
      m_triggerUTime(rawKLM->GetTTUtime(entry)),
      m_windowStart(rawKLM->GetTrailerChksum(entry)),
      m_nRPCHits(0),
      m_nSciHits(0),
      m_nOutOfRangeHits(0),
      m_Revo9TriggerWord(0),
      m_UserWord(0)
    {
    }

    /**
     * Get trigger CTIME.
     * @return Trigger CTIME.
     */
    unsigned int getTriggerCTime() const
    {
      return m_TriggerCTime;
    }

    /**
     * Returns trigger CTIME as int.
     * @return Trigger CTIME.
     */
    int getIntTriggerCTime() const
    {
      return (int)m_TriggerCTime;
    }

    /**
     * Set trigger CTIME.
     * @param[in] triggerCTime Trigger CTIME.
     */
    void setTriggerCTime(unsigned int triggerCTime)
    {
      m_TriggerCTime = triggerCTime;
    }

    /**
     * Returns trigger interval (triggerCTime - triggerCTimeOfPreviousEvent)
     * @return trigger interval
     */
    long int getTriggerInterval() const
    {
      return (long int)m_TriggerCTime - (long int)m_triggerCTimeOfPreviousEvent;
    }

    /**
     * Returns trigger interval (triggerCTime - triggerCTimeOfPreviousEvent)
     * in us.
     * @return trigger interval in us
     */
    double getTriggerIntervalInUs() const
    {
      return ((long int)m_TriggerCTime -
              (long int)m_triggerCTimeOfPreviousEvent) / 127.216;
    }

    /**
     * Set trigger CTime of previous event
     */
    void setPreviousEventTriggerCTime(unsigned int triggerCTimeOfPreviousEvent)
    {
      m_triggerCTimeOfPreviousEvent = triggerCTimeOfPreviousEvent;
    }

    /**
     * Get trigger UTIME.
     * @return Trigger UTIME.
     */
    unsigned int getTriggerUTime() const
    {
      return m_triggerUTime;
    }

    /**
     * Set trigger UTIME.
     */
    void setTriggerUTime(unsigned int triggerUTime)
    {
      m_triggerUTime = triggerUTime;
    }

    /**
     * Get window start.
     * @return Window start.
     */
    unsigned int getWindowStart() const
    {
      return m_windowStart;
    }

    /**
     * Set window start.
     */
    void setWindowStart(unsigned int windowStart)
    {
      m_windowStart = windowStart;
    }

    /**
     * Returns the number of RPC hits in the event.
     * @return nRPCHits
     */
    int getRPCHits() const
    {
      return m_nRPCHits;
    }

    /**
     * Increase by 1 the number of RPC hits in the event.
     */
    void increaseRPCHits()
    {
      m_nRPCHits++;
    }

    /**
     * Increase by 'hits' the number of RPC hits in the event.
     */
    void increaseRPCHits(int hits)
    {
      m_nRPCHits += hits;
    }

    /**
     * Returns the number of scintillator hits in the event.
     * @return nSciHits
     */
    int getSciHits() const
    {
      return m_nSciHits;
    }

    /**
     * Increase by 1 the number of scintillator hits in the event.
     */
    void increaseSciHits()
    {
      m_nSciHits++;
    }

    /**
     * Increase by 'hits' the number of scintillator hits in the event.
     */
    void increaseSciHits(int hits)
    {
      m_nSciHits += hits;
    }

    /**
     * Returns the number of OutOfRange-flagged hits in the event
     * @return nOutOfRangeHits
     */
    int getOutOfRangeHits() const
    {
      return m_nOutOfRangeHits;
    }

    /**
     * Increase by 1 the number of outOfRange-flagged hits in the event.
     */
    void increaseOutOfRangeHits()
    {
      m_nOutOfRangeHits++;
    }

    /**
     * Increase by 'hits' the number of outOfRange-flagged hits in the event.
     */
    void increaseOutOfRangeHits(int hits)
    {
      m_nOutOfRangeHits += hits;
    }

    /**
     * Get revo9 trigger word (from DCs).
     * @return Revo9 trigger word (from DCs).
     */
    unsigned int getRevo9TriggerWord() const
    {
      return m_Revo9TriggerWord;
    }

    /**
     * Set Revo9 trigger word (from DCs).
     * @param[in] revo9TriggerWord Revo9 trigger word (from DCs).
     */
    void setRevo9TriggerWord(unsigned int revo9TriggerWord)
    {
      m_Revo9TriggerWord = revo9TriggerWord;
    }

    /**
     * Get user word (from DCs).
     * @return User word (from DCs).
     */
    unsigned int getUserWord() const
    {
      return m_UserWord;
    }

    /**
     * Set user word (from DCs).
     * @param[in] userWord User word (from DCs).
     */
    void setUserWord(unsigned int userWord)
    {
      m_UserWord = userWord;
    }

  protected:

    /** Trigger CTime */
    unsigned int m_TriggerCTime;

    /** Trigger CTime of previous event. */
    unsigned int m_triggerCTimeOfPreviousEvent;

    /** Trigger UTime. */
    unsigned int m_triggerUTime;

    /** Window start. */
    unsigned int m_windowStart;

    /** RPC hits in the event. */
    int m_nRPCHits;

    /** Scintillator hits in the event. */
    int m_nSciHits;

    /** Out-of-range hits (skipped hits with layer > 14 for BKLM). */
    int m_nOutOfRangeHits;

    /** Revo9 trigger word (from DCs). */
    unsigned int m_Revo9TriggerWord;

    /** User word (from DCs) */
    unsigned int m_UserWord;

    /** Class version. */
    ClassDef(KLMDigitEventInfo, 5);

  };

}
