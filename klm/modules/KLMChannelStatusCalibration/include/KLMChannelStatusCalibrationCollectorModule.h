/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>

namespace Belle2 {

  /**
   * KLM channel status calibration (data collection).
   */
  class KLMChannelStatusCalibrationCollectorModule :
    public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    KLMChannelStatusCalibrationCollectorModule();

    /**
     * Destructor.
     */
    ~KLMChannelStatusCalibrationCollectorModule();

    /**
     * Initializer.
     */
    void prepare() override;

    /**
     * This method is called for each event.
     */
    void collect() override;

    /**
     * This method is called at the end of run.
     */
    void closeRun() override;

    /**
     * Collection of data from DQM modules.
     * @param[in] dqmFile File with DQM histograms.
     */
    void collectFromDQM(const char* dqmFile);

  private:

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** KLM channel array index. */
    const KLMChannelArrayIndex* m_ChannelArrayIndex;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_BKLMDigits;

    /** EKLM digits. */
    StoreArray<EKLMDigit> m_EKLMDigits;

    /** Hit map. */
    StoreObjPtr< KLMChannelMapValue<unsigned int> > m_HitMap;

  };

}
