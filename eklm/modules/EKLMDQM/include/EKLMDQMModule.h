/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TH1F.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Module EKLMDigitizationModule.
   * @details
   * Simple module for reading EKLM hits.
   */
  class EKLMDQMModule : public HistoModule {

  public:

    /**
     * Constructor.
     */
    EKLMDQMModule();

    /**
     * Destructor
     */
    virtual ~EKLMDQMModule();

    /**
     * Definition of the histograms.
     */
    virtual void defineHisto() override;

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

    /** Directory for EKLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryName;

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_Elements;

    /** Digits. */
    StoreArray<EKLMDigit> m_Digits;

    /** Sector number. */
    TH1F* m_Sector;

    /** Hit time. */
    TH1F* m_Time;

    /** Strip number within a layer. */
    TH1F** m_StripLayer;

  };

}
