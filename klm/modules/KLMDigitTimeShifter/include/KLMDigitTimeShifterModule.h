/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
