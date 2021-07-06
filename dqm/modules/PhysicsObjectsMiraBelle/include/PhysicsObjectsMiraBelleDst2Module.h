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
