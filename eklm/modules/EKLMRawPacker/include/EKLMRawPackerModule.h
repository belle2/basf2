/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRAWPACKER_H
#define EKLMRAWPACKER_H

/* C++ headers. */
#include <cstdint>
#include <map>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawKLM.h>

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

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

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

#endif

