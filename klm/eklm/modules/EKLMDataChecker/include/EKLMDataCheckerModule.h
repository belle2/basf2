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
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

/* C++ headers. */
#include <map>

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
    ~EKLMDataCheckerModule();

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

    /** Element numbers. */
    const EKLMElementNumbers* m_ElementNumbers;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Digits. */
    StoreArray<KLMDigit> m_Digits;

    /** Map of strip data information. */
    std::map<int, struct StripData> m_StripDataMap;

  };

}
