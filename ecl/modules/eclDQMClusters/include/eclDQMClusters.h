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
#include <TProfile.h>

namespace Belle2 {

  /**
   * This module is created to monitor ECL Data Quality.
   */
  class ECLDQMClustersModule : public HistoModule {  /**< derived from HistoModule class. */

  public:

    /** Constructor. */
    ECLDQMClustersModule();

    /** Destructor. */
    virtual ~ECLDQMClustersModule();

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Call when a run ends. */
    virtual void endRun() override;
    /** Terminate. */
    virtual void terminate() override;

    /** Function to define histograms. */
    virtual void defineHisto() override;

  private:

    /** Histogram to hold average number of crystals in Clusters */
    TProfile* m_CrystalsInClustersHistogram;
    /** Histogram to hold number of bad Clusters */
    TProfile* m_BadClustersHistogram;
    /** Histogram to hold crystals distribution in ECL Clusters */
    TH1F* m_ClustersCrystalsNumHistogram;
    /** Crystals number threshold */
    int m_BadCrystalsThreshold;
    /** Histogram directory in ROOT file */
    std::string m_histogramDirectoryName;
  };
}; // end Belle2 namespace
