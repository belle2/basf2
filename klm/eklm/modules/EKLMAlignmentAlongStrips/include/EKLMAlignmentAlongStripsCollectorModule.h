/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/eklm/calibration/EKLMAlignmentAlongStripsAlgorithm.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>

/* Belle 2 headers. */
#include <calibration/CalibrationCollectorModule.h>
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

    /** Element numbers. */
    const EKLMElementNumbers* m_ElementNumbers;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** EKLM digits. */
    StoreArray<KLMDigit> m_KLMDigits;

    /** Tracks. */
    StoreArray<Track> m_Tracks;

    /** Event. */
    struct EKLMAlignmentAlongStripsAlgorithm::Event* m_Event;

  };

}
