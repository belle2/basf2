/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDATACHECKERMODULE_H
#define EKLMDATACHECKERMODULE_H

/* C++ headers. */
#include <map>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Module for checking of collected data.
   */
  class EKLMDataCheckerModule : public Module {

  public:

    /**
     * Strip data information.
     */
    struct StripData {
      int strip;      /**< Strip global number. */
      int nDigits;    /**< Total number of digits. */
      int nBadDigits; /**< Number of digits with readout error. */
    };

    /**
     * Constructor.
     */
    EKLMDataCheckerModule();

    /**
     * Destructor.
     */
    virtual ~EKLMDataCheckerModule();

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

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Digits. */
    StoreArray<EKLMDigit> m_Digits;

    /** Map of strip data information. */
    std::map<int, struct StripData> m_StripDataMap;

  };

}

#endif

