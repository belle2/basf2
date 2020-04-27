/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMHit2d.h>
#include <klm/eklm/calibration/EKLMTimeCalibrationAlgorithm.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>

/* Belle 2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>

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
    void prepare() override;

    /**
     * This method is called for each event.
     */
    void collect() override;

    /**
     * This method is called at the end of the event processing.
     */
    void finish() override;

  private:

    /** Use enent T0 or not. */
    bool m_UseEventT0;

    /** Element numbers. */
    const EKLMElementNumbers* m_ElementNumbers;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** EKLM 2d hits. */
    StoreArray<EKLMHit2d> m_EKLMHit2ds;

    /** Tracks. */
    StoreArray<Track> m_Tracks;

    /** Event T0. */
    StoreObjPtr<EventT0> m_EventT0;

    /** Event (for tree branches). */
    struct EKLMTimeCalibrationAlgorithm::Event m_ev;

    /** Number of strip (for tree branch). */
    int m_Strip;

  };

}
