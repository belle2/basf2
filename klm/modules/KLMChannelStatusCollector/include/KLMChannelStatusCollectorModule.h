/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMChannelMapValue.h>
#include <klm/dataobjects/KLMElementNumbers.h>

/* Belle 2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /**
   * KLM channel status calibration (data collection).
   */
  class KLMChannelStatusCollectorModule :
    public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    KLMChannelStatusCollectorModule();

    /**
     * Destructor.
     */
    ~KLMChannelStatusCollectorModule();

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

    /** KLM digits. */
    StoreArray<KLMDigit> m_KLMDigits;

    /** Hit map. */
    StoreObjPtr< KLMChannelMapValue<unsigned int> > m_HitMap;

  };

}
