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
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
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

    /** Use default elect id, if not found in mapping file. */
    bool m_useDefaultElectId;

    /** Number of events in a run. */
    int m_MaxNEvents;

    /** Number of events. */
    int m_NEvents;

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** RawKLM array. */
    StoreArray<RawKLM> m_RawKLMArray;

    /** Map of logical coordinates to hardware coordinates. */
    std::map<int, int> m_ModuleIdToelectId;

    /**
     * Creation of raw data.
     * @param[in]  endcap Endcap number.
     * @param[in]  layer  Layer number.
     * @param[in]  sector Sector number.
     * @param[in]  plane  Plane number.
     * @param[in]  strip  Strip number.
     * @param[in]  charge Charge.
     * @param[in]  ctime  Time.
     * @param[out] bword1 First word.
     * @param[out] bword2 Second word.
     * @param[out] bword3 Third word.
     * @param[out] bword4 Fourth word.
     */
    void formatData(int endcap, int layer, int sector, int plane, int strip,
                    int charge, float ctime, uint16_t& bword1,
                    uint16_t& bword2, uint16_t& bword3, uint16_t& bword4);

    /**
     * Fill m_ModuleIdToelectId.
     */
    void loadMap();

  };

}

#endif

