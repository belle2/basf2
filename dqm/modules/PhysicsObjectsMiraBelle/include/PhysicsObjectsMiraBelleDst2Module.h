/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shun Watanuki                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    PhysicsObjectsMiraBelleDst2Module();

    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

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
