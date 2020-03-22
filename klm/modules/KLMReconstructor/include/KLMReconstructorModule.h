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
#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/bklm/dataobjects/BKLMHit1d.h>
#include <klm/bklm/dataobjects/BKLMHit2d.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/dataobjects/EKLMAlignmentHit.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <klm/eklm/dbobjects/EKLMReconstructionParameters.h>
#include <klm/eklm/dbobjects/EKLMTimeCalibration.h>
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
     * Get 2d hit time corresponding to EKLMDigit.
     * @param[in] d    EKLMDigit.
     * @param[in] dist Distance from 2d hit to SiPM.
     */
    double getTime(EKLMDigit* d, double dist);

    /* BKLM parameters. */

    /** BKLM GeometryPar singleton. */
    bklm::GeometryPar* m_bklmGeoPar;

    /** Half-width time coincidence window between adjacent BKLMDigits or orthogonal BKLMHit1ds (ns). */
    double m_bklmCoincidenceWindow;

    /** Nominal time of prompt BKLMHit2ds (ns). */
    double m_bklmPromptTime;

    /** Half-width window of BKLMHit2ds relative to m_PrompTime (ns). */
    double m_bklmPromptWindow;

    /** Perform alignment correction (true) or not (false). */
    bool m_bklmIfAlign;

    /** Load timing window from database (true) or not (false). */
    bool m_bklmLoadTimingFromDB;

    /** Ignore scintillators (to debug their electronics mapping). */
    bool m_bklmIgnoreScintillators;

    /** BKLM time window. */
    DBObjPtr<BKLMTimeWindow> m_bklmTiming;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_bklmDigits;

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

    /** Digits. */
    StoreArray<EKLMDigit> m_eklmDigits;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_eklmHit2ds;

    /** Alignment Hits. */
    StoreArray<EKLMAlignmentHit> m_eklmAlignmentHits;

  };
} // end namespace Belle2
