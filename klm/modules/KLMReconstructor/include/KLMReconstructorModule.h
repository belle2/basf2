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
   * Create BKLMHit1ds from BKLMDigits and then create BKLMHit2ds from BKLMHit1ds.
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

    /**
     * Check whether hit has too small time.
     * @param[in] pos  Hit position.
     * @param[in] time HIt time.
     */
    bool fastHit(HepGeom::Point3D<double>& pos, double time);

    /* BKLM parameters. */

    /** BKLM GeometryPar singleton. */
    bklm::GeometryPar* m_BklmGeoPar;

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

    /** BKLM time window. */
    DBObjPtr<BKLMTimeWindow> m_Timing;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_BklmDigits;

    /** BKLM 1d hits. */
    StoreArray<BKLMHit1d> m_BklmHit1ds;

    /** BKLM 2d hits. */
    StoreArray<BKLMHit2d> m_BklmHit2ds;

    /* EKLM parameters. */

    /**
     * Check if segments intersect. Normally should be true, but it may be
     * necessary to turn this check off for debugging.
     */
    bool m_CheckSegmentIntersection;

    /** Geometry data. */
    const EKLM::GeometryData* m_EklmGeoDat;

    /** Number of strips. */
    int m_nStrip;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** Reconstruction parameters. */
    DBObjPtr<EKLMReconstructionParameters> m_RecPar;

    /** Time calibration data. */
    DBObjPtr<EKLMTimeCalibration> m_TimeCalibration;

    /** Time calibration data for individual strips. */
    const EKLMTimeCalibrationData** m_TimeCalibrationData;

    /** Default time calibration data. */
    EKLMTimeCalibrationData m_DefaultTimeCalibrationData;

    /** Digits. */
    StoreArray<EKLMDigit> m_EklmDigits;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_EklmHit2ds;

    /** Alignment Hits. */
    StoreArray<EKLMAlignmentHit> m_EklmAlignmentHits;

  };
} // end namespace Belle2
