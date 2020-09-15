/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shun Watanuki                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <string>
#include "TH1F.h"

namespace Belle2 {
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class PhysicsObjectsMiraBelleDstModule : public HistoModule {
  public:
    /** Constructor, for setting module description and parameters. */
    PhysicsObjectsMiraBelleDstModule();

    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    void defineHisto() override;

  private:
    // ======== Hitograms for run dependence
    TH1F* m_h_D0_InvM = nullptr;
    TH1F* m_h_delta_m = nullptr;
    TH1F* m_h_D0_softpi_PID_ALL_pion = nullptr;
    TH1F* m_h_D0_softpi_PID_SVD_pion = nullptr;
    TH1F* m_h_D0_softpi_PID_CDC_pion = nullptr;
    TH1F* m_h_D0_softpi_PID_TOP_pion = nullptr;
    TH1F* m_h_D0_softpi_PID_ARICH_pion = nullptr;
    TH1F* m_h_D0_softpi_PID_ECL_pion = nullptr;
    TH1F* m_h_D0_softpi_PID_KLM_pion = nullptr;
    TH1F* m_h_D0_pi_PID_ALL_pion = nullptr;
    TH1F* m_h_D0_pi_PID_SVD_pion = nullptr;
    TH1F* m_h_D0_pi_PID_CDC_pion = nullptr;
    TH1F* m_h_D0_pi_PID_TOP_pion = nullptr;
    TH1F* m_h_D0_pi_PID_ARICH_pion = nullptr;
    TH1F* m_h_D0_pi_PID_ECL_pion = nullptr;
    TH1F* m_h_D0_pi_PID_KLM_pion = nullptr;
    TH1F* m_h_D0_K_PID_ALL_kaon = nullptr;
    TH1F* m_h_D0_K_PID_SVD_kaon = nullptr;
    TH1F* m_h_D0_K_PID_CDC_kaon = nullptr;
    TH1F* m_h_D0_K_PID_TOP_kaon = nullptr;
    TH1F* m_h_D0_K_PID_ARICH_kaon = nullptr;
    TH1F* m_h_D0_K_PID_ECL_kaon = nullptr;
    TH1F* m_h_D0_K_PID_KLM_kaon = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_ALL_pion = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_SVD_pion = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_CDC_pion = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_TOP_pion = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_ARICH_pion = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_ECL_pion = nullptr;
    TH1F* m_h_sideband_D0_softpi_PID_KLM_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_ALL_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_SVD_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_CDC_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_TOP_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_ARICH_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_ECL_pion = nullptr;
    TH1F* m_h_sideband_D0_pi_PID_KLM_pion = nullptr;
    TH1F* m_h_sideband_D0_K_PID_ALL_kaon = nullptr;
    TH1F* m_h_sideband_D0_K_PID_SVD_kaon = nullptr;
    TH1F* m_h_sideband_D0_K_PID_CDC_kaon = nullptr;
    TH1F* m_h_sideband_D0_K_PID_TOP_kaon = nullptr;
    TH1F* m_h_sideband_D0_K_PID_ARICH_kaon = nullptr;
    TH1F* m_h_sideband_D0_K_PID_ECL_kaon = nullptr;
    TH1F* m_h_sideband_D0_K_PID_KLM_kaon = nullptr;
    TH1F* m_h_D0_pi0_InvM = nullptr;

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the mu+ particle list */
    std::string m_dstListName = "";

  };
}
