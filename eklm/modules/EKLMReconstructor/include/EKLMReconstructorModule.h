/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECONSTRUCTORMODULE_H
#define EKLMRECONSTRUCTORMODULE_H

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMAlignmentHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dbobjects/EKLMReconstructionParameters.h>
#include <eklm/dbobjects/EKLMTimeCalibration.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/TransformData.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Module EKLMReconstructorModule.
   * @details
   * Simple module for reading EKLM hits.
   */
  class EKLMReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMReconstructorModule();

    /**
     * Destructor.
     */
    virtual ~EKLMReconstructorModule();

    /**
     * Initializer.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * This method is called for each event.
     */
    virtual void event();

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate();

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

  private:

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

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
    StoreArray<EKLMDigit> m_Digits;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_Hit2ds;

    /** Alignment Hits. */
    StoreArray<EKLMAlignmentHit> m_AlignmentHits;

  };

}

#endif

