/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Kirill Chilikin, Giacomo De Pietro,               *
 *               Leo Piilonen, Timofey Uglov                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMAlignmentHit.h>
#include <klm/dataobjects/eklm/EKLMHit2d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dbobjects/eklm/EKLMReconstructionParameters.h>
#include <klm/dbobjects/KLMTimeCableDelay.h>
#include <klm/dbobjects/KLMTimeConstants.h>
#include <klm/dbobjects/KLMTimeWindow.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Create BKLMHit1ds from BKLMDigits and then create BKLMHit2ds from BKLMHit1ds;
   * create EKLMHit2ds from EKLMDigits.
   */
  class KLMReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMReconstructorModule();

    /**
     * Destructor.
     */
    virtual ~KLMReconstructorModule();

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

    /* Methods. */

    /**
     * Reconstruct BKLMHit1d and BKLMHit2d.
     */
    void reconstructBKLMHits();

    /**
     * Reconstruct EKLMHit2d.
     */
    void reconstructEKLMHits();

    /**
     * Time correction by subtract cable delay.
     * @param[in] td    Original time of the digit.
     * @param[in] digit KLM Digit.
     */
    void correctCableDelay(double& td, const KLMDigit* digit);


    /* Member variables. */

    /* Parameters and switchs . */
    /* switchs. */
    /** Perform cable delay time correction (true) or not (false). */
    bool m_timeCableDelayCorrection;

    /** Perform alignment correction (true) or not (false). */
    bool m_bklmIfAlign;

    /** Ignore scintillators (to debug their electronics mapping). */
    bool m_bklmIgnoreScintillators;

    /**
     * Check if segments intersect. Normally should be true, but it may be
     * necessary to turn this check off for debugging.
     */
    bool m_eklmCheckSegmentIntersection;

    /* parameters. */
    /** KLM time window. */
    DBObjPtr<KLMTimeWindow> m_TimeWindow;

    /** Half-width of the time coincidence window used to create a 2D hit from 1D digits/hits. */
    double m_CoincidenceWindow;

    /** Nominal time of prompt BKLMHit2ds. */
    double m_PromptTime;

    /** Half-width of the time window relative to the prompt time for BKLMHit2ds. */
    double m_PromptWindow;

    /** Constant used for time information correction. */
    DBObjPtr<KLMTimeConstants> m_timeConstants;

    /** Calibration constant of time delay from cable. */
    DBObjPtr<KLMTimeCableDelay> m_timeCableDelay;

    /** BKLM GeometryPar singleton. */
    bklm::GeometryPar* m_bklmGeoPar;

    /** Element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** Geometry data. */
    const EKLM::GeometryData* m_eklmGeoDat;

    /** Transformation data. */
    EKLM::TransformData* m_eklmTransformData;

    /** Reconstruction parameters. */
    DBObjPtr<EKLMReconstructionParameters> m_eklmRecPar;

    /** Effective light speed in fiber for EKLM scintillators. */
    double m_effC_eklm;

    /** Effective light speed in fiber for BKLM scintillators. */
    double m_effC_bklm;

    /** Effective light speed in fiber for BKLM RPCs. */
    double m_effC_RPC;

    /** Number of strips of EKLM. */
    int m_eklmNStrip;


    /* Input and output storeArrays. */
    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /** BKLM 1d hits. */
    StoreArray<BKLMHit1d> m_bklmHit1ds;

    /** BKLM 2d hits. */
    StoreArray<BKLMHit2d> m_bklmHit2ds;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_eklmHit2ds;

    /** Alignment Hits. */
    StoreArray<EKLMAlignmentHit> m_eklmAlignmentHits;
  };
} // end namespace Belle2
