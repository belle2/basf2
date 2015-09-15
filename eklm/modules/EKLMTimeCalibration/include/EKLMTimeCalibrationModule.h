/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECALIBRATIONMODULE_H
#define EKLMTIMECALIBRATIONMODULE_H

/* C++ headers. */
#include <string>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <calibration/CalibrationModule.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/TransformData.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMTimeCalibrationModule.
   * @details
   * Module for generation of transformation and alignment data.
   */
  class EKLMTimeCalibrationModule : public calibration::CalibrationModule {

    /**
     * Event (hit): time, distance from hit to SiPM.
     */
    struct event {
      float time;   /**< Time. */
      float dist;   /**< Distance. */
    };

  public:

    /**
     * Constructor.
     */
    EKLMTimeCalibrationModule();

    /**
     * Destructor.
     */
    ~EKLMTimeCalibrationModule();

    /**
     * Initializer.
     */
    void Prepare();

    /**
     * This method is called for each event.
     */
    void CollectData();

    /**
     * Close files opened in parallel event processes.
     */
    void closeParallelFiles();

    /**
     * Calibration.
     */
    calibration::CalibrationModule::ECalibrationModuleResult Calibrate();

    /**
     * Monitor calibration results.
     */
    calibration::CalibrationModule::ECalibrationModuleMonitoringResult
    Monitor();

    /**
     * Store calibration results.
     */
    bool StoreInDataBase();

  private:

    /** Perform data collection. */
    bool m_performDataCollection;

    /** Perform calibration. */
    bool m_performCalibration;

    /** Output file name. */
    std::string m_dataOutputFileName;

    /** Output file name. */
    std::string m_calibrationOutputFileName;

    /** Output file */
    TFile* m_outputFile;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Number of strips with different lengths. */
    int m_nStripDifferent;

    /** Trees. */
    TTree** m_Tree;

    /** Event (for tree branches). */
    struct event m_ev;

  };

}

#endif

