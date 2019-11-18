/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/dbobjects/EKLMElectronicsMap.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawKLM.h>

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * EKLM raw data packer.
   */
  class EKLMRawPackerModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMRawPackerModule();

    /**
     * Destructor.
     */
    virtual ~EKLMRawPackerModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

  private:

    /**
     * Creation of raw data.
     * @param[in]  lane   Data concentrator lane.
     * @param[in]  plane  Plane number.
     * @param[in]  strip  Strip number.
     * @param[in]  charge Charge.
     * @param[in]  ctime  CTine.
     * @param[in]  tdc    TDC.
     * @param[out] bword1 First word.
     * @param[out] bword2 Second word.
     * @param[out] bword3 Third word.
     * @param[out] bword4 Fourth word.
     */
    void formatData(const EKLMDataConcentratorLane* lane,
                    int plane, int strip, int charge,
                    uint16_t ctime, uint16_t tdc,
                    uint16_t& bword1, uint16_t& bword2, uint16_t& bword3,
                    uint16_t& bword4);

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /** Electronics map. */
    DBObjPtr<EKLMElectronicsMap> m_ElectronicsMap;

    /** Number of events. */
    int m_NEvents;

    /** Raw data. */
    StoreArray<RawKLM> m_RawKLMs;

    /** Digits. */
    StoreArray<EKLMDigit> m_Digits;

  };

}
