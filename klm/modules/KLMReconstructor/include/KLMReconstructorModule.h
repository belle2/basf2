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
#include <klm/dbobjects/eklm/EKLMTimeCalibration.h>
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

    /**
     * Reconstruct BKLMHit1d and BKLMHit2d.
     */
    void reconstructBKLMHits();

    /**
     * Reconstruct EKLMHit2d.
     */
    void reconstructEKLMHits();

    /* EKLM methods. */

    /**
     * Get 2d hit time corresponding to EKLM digit.
     * @param[in] d    EKLM Digit.
     * @param[in] dist Distance from 2d hit to SiPM.
     */
    double getTime(KLMDigit* d, double dist);

    /** Half-width of the time coincidence window used to create a 2D hit from 1D digits/hits. */
    double m_CoincidenceWindow;

    /** Nominal time of prompt BKLMHit2ds. */
    double m_PromptTime;

    /** Half-width of the time window relative to the prompt time for BKLMHit2ds. */
    double m_PromptWindow;

    /** KLM time window. */
    DBObjPtr<KLMTimeWindow> m_TimeWindow;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /* BKLM parameters. */

    /** BKLM GeometryPar singleton. */
    bklm::GeometryPar* m_bklmGeoPar;

    /** Perform alignment correction (true) or not (false). */
    bool m_bklmIfAlign;

    /** Ignore scintillators (to debug their electronics mapping). */
    bool m_bklmIgnoreScintillators;

    /** BKLM 1d hits. */
    StoreArray<BKLMHit1d> m_bklmHit1ds;

    /** BKLM 2d hits. */
    StoreArray<BKLMHit2d> m_bklmHit2ds;

    /* EKLM parameters. */

    /**
     * Check if segments intersect. Normally should be true, but it may be
     * necessary to turn this check off for debugging.
     */
    bool m_eklmCheckSegmentIntersection;

    /** Element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** Geometry data. */
    const EKLM::GeometryData* m_eklmGeoDat;

    /** Number of strips. */
    int m_eklmNStrip;

    /** Transformation data. */
    EKLM::TransformData* m_eklmTransformData;

    /** Reconstruction parameters. */
    DBObjPtr<EKLMReconstructionParameters> m_eklmRecPar;

    /** Time calibration data. */
    DBObjPtr<EKLMTimeCalibration> m_eklmTimeCalibration;

    /** Time calibration data for individual strips. */
    const EKLMTimeCalibrationData** m_eklmTimeCalibrationData;

    /** Default time calibration data. */
    EKLMTimeCalibrationData m_eklmDefaultTimeCalibrationData;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_eklmHit2ds;

    /** Alignment Hits. */
    StoreArray<EKLMAlignmentHit> m_eklmAlignmentHits;

  };
} // end namespace Belle2
