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
    KLMDigitEventInfo() : m_TriggerCTime(0) {};

    /**
     * Get trigger CTIME.
     * @return Trigger CTIME.
     */
    unsigned int getTriggerCTime() const { return m_TriggerCTime; }

    /**
     * Set trigger CTIME.
     * @param[in] ctime Trigger CTime
     */
    void setTriggerCTime(unsigned int triggerCTime) { m_TriggerCTime = triggerCTime; }

  protected:

    /** Trigger CTime */
    unsigned int m_TriggerCTime;

    /** Class version. */
    ClassDef(KLMDigitEventInfo, 1);

  };

}
