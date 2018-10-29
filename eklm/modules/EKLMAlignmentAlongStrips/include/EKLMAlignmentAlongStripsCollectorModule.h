/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTALONGSTRIPSCOLLECTORMODULE_H
#define EKLMALIGNMENTALONGSTRIPSCOLLECTORMODULE_H

/* C++ headers. */
#include <string>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <eklm/calibration/EKLMAlignmentAlongStripsAlgorithm.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/TransformData.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {

  /**
   * EKLM time calibration (data collection).
   */
  class EKLMAlignmentAlongStripsCollectorModule :
    public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    EKLMAlignmentAlongStripsCollectorModule();

    /**
     * Copy constructor (disabled).
     */
    EKLMAlignmentAlongStripsCollectorModule(
      const EKLMAlignmentAlongStripsCollectorModule&) = delete;

    /**
     * Operator = (disabled).
     */
    EKLMAlignmentAlongStripsCollectorModule& operator=(
      const EKLMAlignmentAlongStripsCollectorModule&) = delete;

    /**
     * Destructor.
     */
    ~EKLMAlignmentAlongStripsCollectorModule();

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

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** EKLM digits. */
    StoreArray<EKLMDigit> m_EKLMDigits;

    /** Tracks. */
    StoreArray<Track> m_Tracks;

    /** Event. */
    struct EKLMAlignmentAlongStripsAlgorithm::Event* m_Event;

  };

}

#endif

