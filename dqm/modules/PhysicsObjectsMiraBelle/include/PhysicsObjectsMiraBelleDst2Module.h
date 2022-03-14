/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
//#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <string>
#include "TH1F.h"

namespace Belle2 {
  /** DQM modules to monitor D* related variables */
  class PhysicsObjectsMiraBelleDst2Module : public HistoModule {
  public:

    /**
     * Constructor.
     */
    PhysicsObjectsMiraBelleDst2Module();

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

    /** Pi0 invariant mass for D0->Kpipi0 */
    TH1F* m_h_D0_pi0_InvM = nullptr;

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the mu+ particle list */
    std::string m_dstListName = "";

  };
}
