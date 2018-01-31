/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMUNPACKERMODULE_H
#define EKLMUNPACKERMODULE_H

/* C++ headers. */
#include <string>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <eklm/dbobjects/EKLMTimeConversion.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawKLM.h>

namespace Belle2 {

  /**
   * EKLM unpacker.
   */
  class EKLMUnpackerModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMUnpackerModule();

    /**
     * Destructor.
     */
    virtual ~EKLMUnpackerModule();

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

    /** Name of EKLMDigit store array. */
    std::string m_outputDigitsName;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Electronics map. */
    DBObjPtr<EKLMElectronicsMap> m_ElectronicsMap;

    /** Time conversion. */
    DBObjPtr<EKLMTimeConversion> m_TimeConversion;

    /** Digits. */
    StoreArray<EKLMDigit> m_Digits;

    /** Raw data. */
    StoreArray<RawKLM> m_RawKLMs;

  };

}

#endif

