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
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMElectronicsMap.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawKLM.h>

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * KLM raw packer.
   */
  class KLMPackerModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMPackerModule();

    /**
     * Destructor.
     */
    ~KLMPackerModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /**
     * Creation of raw data.
     * @param[in]  flag   Flag for RPCs or scintillators.
     * @param[in]  lane   Data concentrator lane.
     * @param[in]  plane  Plane number.
     * @param[in]  strip  Strip number.
     * @param[in]  charge Charge.
     * @param[in]  ctime  CTime.
     * @param[in]  tdc    TDC.
     * @param[out] bword1 First word.
     * @param[out] bword2 Second word.
     * @param[out] bword3 Third word.
     * @param[out] bword4 Fourth word.
     */
    void formatData(int flag, int lane, int plane, int strip, int charge, uint16_t ctime, uint16_t tdc,
                    uint16_t& bword1, uint16_t& bword2, uint16_t& bword3, uint16_t& bword4);

    /* Common objects. */

    /** Raw data. */
    StoreArray<RawKLM> m_RawKLMs;

    /** Event meta data. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

    /** Electronics map. */
    DBObjPtr<KLMElectronicsMap> m_ElectronicsMap;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

  };

}
