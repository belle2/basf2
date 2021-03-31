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
#include <klm/dbobjects/KLMChannelStatus.h>
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
    ~KLMReconstructorModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * Called for each event.
     */
    void event() override;

    /**
     * Called if the current run ends.
     */
    void endRun() override;

    /**
     * Called at the end of the event processing.
     */
    void terminate() override;

  private:

    /* Functions. */

    /**
     * Reconstruct BKLMHit1d and BKLMHit2d.
     */
    void reconstructBKLMHits();

    /**
     * Reconstruct EKLMHit2d.
     */
    void reconstructEKLMHits();

    /**
     * Check if channel is normal or dead. Dead channels should not
     * contain any signal; they are allowed for debugging.
     * @param[in] digit KLM digit.
     */
    bool isNormal(const KLMDigit* digit) const;

    /**
     * Get 2d hit time corresponding to EKLM digit.
     * @param[in] d    EKLM Digit.
     * @param[in] dist Distance from 2d hit to SiPM.
     */
    double getTime(KLMDigit* d, double dist);

    /* Common member variables. */

    /**
     * Half-width of the time coincidence window used to create a 2D hit
     * from 1D digits/hits.
     */
    double m_CoincidenceWindow;

    /** Nominal time of prompt BKLMHit2ds. */
    double m_PromptTime;

    /**
     * Half-width of the time window relative to the prompt time
     * for BKLMHit2ds.
     */
    double m_PromptWindow;

    /**
     * Use only normal and dead (for debugging) channels during 2d hit
     * reconstruction.
     */
    bool m_IgnoreHotChannels;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** KLM time window. */
    DBObjPtr<KLMTimeWindow> m_TimeWindow;

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /* BKLM member variables. */

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

    /* EKLM member variables. */

    /**
     * Check if segments intersect. Normally should be true, but it may be
     * necessary to turn this check off for debugging.
     */
    bool m_eklmCheckSegmentIntersection;

    /** EKLM element numbers. */
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

}
