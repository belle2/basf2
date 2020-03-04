/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/bklm/dataobjects/BKLMHit1d.h>
#include <klm/bklm/dataobjects/BKLMHit2d.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/KLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Create BKLMHit1ds from BKLMDigits and then create BKLMHit2ds from BKLMHit1ds.
   */
  class BKLMReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    BKLMReconstructorModule();

    /**
     * Destructor.
     */
    virtual ~BKLMReconstructorModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * Called at the end of the event processing.
     */
    virtual void terminate() override;

  private:

    /** BKLM GeometryPar singleton. */
    bklm::GeometryPar* m_GeoPar;

    /** Half-width time coincidence window between adjacent BKLMDigits or orthogonal BKLMHit1ds (ns). */
    double m_CoincidenceWindow;

    /** Nominal time of prompt BKLMHit2ds (ns). */
    double m_PromptTime;

    /** Half-width window of BKLMHit2ds relative to m_PrompTime (ns). */
    double m_PromptWindow;

    /** Perform alignment correction (true) or not (false). */
    bool m_IfAlign;

    /** Load timing window from database (true) or not (false). */
    bool m_LoadTimingFromDB;

    /** Ignore scintillators (to debug their electronics mapping). */
    bool m_IgnoreScintillators;

    /** Ignore this module (to debug its electronics mapping). */
    int m_IgnoreModule;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** BKLM time window. */
    DBObjPtr<BKLMTimeWindow> m_Timing;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_Digits;

    /** BKLM 1d hits. */
    StoreArray<BKLMHit1d> m_Hit1ds;

    /** BKLM 2d hits. */
    StoreArray<BKLMHit2d> m_Hit2ds;

  };
} // end namespace Belle2
