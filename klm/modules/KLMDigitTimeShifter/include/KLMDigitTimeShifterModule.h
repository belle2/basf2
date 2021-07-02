/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dbobjects/KLMTimeCableDelay.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * KLM digit time shifter module.
   */
  class KLMDigitTimeShifterModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMDigitTimeShifterModule();

    /**
     * Destructor.
     */
    ~KLMDigitTimeShifterModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

  private:

    /** KLM time cable delay. */
    DBObjPtr<KLMTimeCableDelay> m_TimeDelay;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

  };

}
