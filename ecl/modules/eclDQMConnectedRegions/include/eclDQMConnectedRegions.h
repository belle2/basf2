/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/core/ModuleParam.templateDetails.h>

/* ROOT headers */
#include <TH1F.h>

namespace Belle2 {

  /**
   * This module is created to monitor ECL Data Quality.
   */
  class ECLDQMConnectedRegionsModule : public HistoModule {  /**< derived from HistoModule class. */

  public:

    /** Constructor. */
    ECLDQMConnectedRegionsModule();

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Function to define histograms. */
    virtual void defineHisto() override;

  private:

    /** Histogram to hold number of crystals in largest ECL connected region */
    TH1F* m_largestCRCrystalsNum;
    /** Histogram to hold number of local maximum in ECL connected region */
    TH1F* m_largestCRLocalMaxNum;
    /** Histogram directory in ROOT file */
    std::string m_histogramDirectoryName;
  };
}; // end Belle2 namespace
