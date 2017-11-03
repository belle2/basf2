/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECALIBRATIONCOLLECTORMODULE_H
#define EKLMTIMECALIBRATIONCOLLECTORMODULE_H

/* C++ headers. */
#include <string>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <eklm/calibration/EKLMTimeCalibrationAlgorithm.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/TransformData.h>

namespace Belle2 {

  /**
   * EKLM time calibration (data collection).
   */
  class EKLMTimeCalibrationCollectorModule :
    public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    EKLMTimeCalibrationCollectorModule();

    /**
     * Destructor.
     */
    ~EKLMTimeCalibrationCollectorModule();

    /**
     * Initializer.
     */
    void prepare();

    /**
     * This method is called for each event.
     */
    void collect();

    /**
     * This method is called at the end of the event processing.
     */
    void terminate();

  private:

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Event (for tree branches). */
    struct EKLMTimeCalibrationAlgorithm::Event m_ev;

    /** Number of strip (for tree branch). */
    int m_Strip;

  };

}

#endif

