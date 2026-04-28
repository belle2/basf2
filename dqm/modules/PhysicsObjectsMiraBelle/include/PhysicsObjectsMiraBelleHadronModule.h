/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <TH1F.h>

#include <string>

namespace Belle2 {
  /** DQM modules to monitor hadron related variables */
  class PhysicsObjectsMiraBelleHadronModule : public HistoModule {
  public:

    /**
     * Constructor.
     */
    PhysicsObjectsMiraBelleHadronModule();

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

    /**
     * Definition of the histograms.
     */
    void defineHisto() override;

  private:

    /*******************************
    * Hitograms for run dependence *
    ********************************/

    /** To check hadron events */
    TH1F* m_h_nECLClusters = nullptr;/**< histogram for number of ecl clusters */
    TH1F* m_h_visibleEnergyCMSnorm = nullptr;/**< histogram for visible energy of tracks and gammas */
    TH1F* m_h_EsumCMSnorm = nullptr; /**< histogram for sum of energy of clusters with E > 0.1*/
    TH1F* m_h_R2 = nullptr; /**< histogram for R2 of hadron events after the hadronb2 selection*/
    TH1F* m_h_physicsresultsH = nullptr; /**< histogram for event results for hadronb2 and hadronb2 tight*/

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the pi+ particle list */
    std::string m_hadpiPListName = "";

  };
}
