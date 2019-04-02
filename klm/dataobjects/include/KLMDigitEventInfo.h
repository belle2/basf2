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
  class KLMDigitEventInfo : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    KLMDigitEventInfo():
      m_TriggerCTime(0),
      m_userWord(0)
    {};

    /**
     * Set trigger CTime.
     * @param[in] triggerCTime Trigger CTime.
     */
    void setTriggerCTime(unsigned int triggerCTime) { m_TriggerCTime = triggerCTime; }

    /**
     * Set user word (from DCs).
     * @param[in] userWord User word (from DCs).
     */
    void setUserWord(unsigned int userWord) { m_userWord = userWord; }

    /**
     * Get trigger CTIME.
     * @return Trigger CTIME.
     */
    unsigned int getTriggerCTime() const { return m_TriggerCTime; }

    /**
     * Get user word (from DCs).
     * @return User word (from DCs).
     */
    unsigned int getUserWord() const { return m_userWord; }

  protected:

    /** Trigger CTime */
    unsigned int m_TriggerCTime;

    /** User word (from DCs) */
    unsigned int m_userWord;

    /** Class version. */
    ClassDef(KLMDigitEventInfo, 3);

  };

}
