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
  class PhysicsObjectsMiraBelleDstModule : public HistoModule {
  public:

    /**
     * Constructor.
     */
    PhysicsObjectsMiraBelleDstModule();

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

    /** D0 invariant mass */
    TH1F* m_h_D0_InvM = nullptr;

    /** Mass difference b/w D*+ and D0 */
    TH1F* m_h_delta_m = nullptr;

    /** PID of all detectors for soft pion */
    TH1F* m_h_D0_softpi_PID_ALL_pion = nullptr;

    /** PID of SVD for soft pion */
    TH1F* m_h_D0_softpi_PID_SVD_pion = nullptr;

    /** PID of CDC for soft pion */
    TH1F* m_h_D0_softpi_PID_CDC_pion = nullptr;

    /** PID of TOP for soft pion */
    TH1F* m_h_D0_softpi_PID_TOP_pion = nullptr;

    /** PID of ARICH for soft pion */
    TH1F* m_h_D0_softpi_PID_ARICH_pion = nullptr;

    /** PID of ECL for soft pion */
    TH1F* m_h_D0_softpi_PID_ECL_pion = nullptr;

    /** PID of KLM for soft pion */
    TH1F* m_h_D0_softpi_PID_KLM_pion = nullptr;

    /** PID of all for pion */
    TH1F* m_h_D0_pi_PID_ALL_pion = nullptr;

    /** PID of SVD for pion */
    TH1F* m_h_D0_pi_PID_SVD_pion = nullptr;

    /** PID of CDC for pion */
    TH1F* m_h_D0_pi_PID_CDC_pion = nullptr;

    /** PID of TOP for pion */
    TH1F* m_h_D0_pi_PID_TOP_pion = nullptr;

    /** PID of ARICH for pion */
    TH1F* m_h_D0_pi_PID_ARICH_pion = nullptr;

    /** PID of ECL for pion */
    TH1F* m_h_D0_pi_PID_ECL_pion = nullptr;

    /** PID of KLM for pion */
    TH1F* m_h_D0_pi_PID_KLM_pion = nullptr;

    /** PID of all detectors for kaon */
    TH1F* m_h_D0_K_PID_ALL_kaon = nullptr;

    /** PID of SVD for kaon */
    TH1F* m_h_D0_K_PID_SVD_kaon = nullptr;

    /** PID of CDC for kaon */
    TH1F* m_h_D0_K_PID_CDC_kaon = nullptr;

    /** PID of TOP for kaon */
    TH1F* m_h_D0_K_PID_TOP_kaon = nullptr;

    /** PID of ARICH for kaon */
    TH1F* m_h_D0_K_PID_ARICH_kaon = nullptr;

    /** PID of ECL for kaon */
    TH1F* m_h_D0_K_PID_ECL_kaon = nullptr;

    /** PID of KLM for kaon */
    TH1F* m_h_D0_K_PID_KLM_kaon = nullptr;

    /** PID of all detectors for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_ALL_pion = nullptr;

    /** PID of SVD for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_SVD_pion = nullptr;

    /** PID of CDC for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_CDC_pion = nullptr;

    /** PID of TOP for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_TOP_pion = nullptr;

    /** PID of ARICH for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_ARICH_pion = nullptr;

    /** PID of ECL for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_ECL_pion = nullptr;

    /** PID of KLM for soft pion in sideband */
    TH1F* m_h_sideband_D0_softpi_PID_KLM_pion = nullptr;

    /** PID of all detectors for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_ALL_pion = nullptr;

    /** PID of SVD for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_SVD_pion = nullptr;

    /** PID of CDC for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_CDC_pion = nullptr;

    /** PID of TOP for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_TOP_pion = nullptr;

    /** PID of ARICH for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_ARICH_pion = nullptr;

    /** PID of ECL for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_ECL_pion = nullptr;

    /** PID of KLM for pion in sideband */
    TH1F* m_h_sideband_D0_pi_PID_KLM_pion = nullptr;

    /** PID of all detectors for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_ALL_kaon = nullptr;

    /** PID of SVD for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_SVD_kaon = nullptr;

    /** PID of CDC for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_CDC_kaon = nullptr;

    /** PID of TOP for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_TOP_kaon = nullptr;

    /** PID of ARICH for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_ARICH_kaon = nullptr;

    /** PID of ECL for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_ECL_kaon = nullptr;

    /** PID of KLM for kaon in sideband */
    TH1F* m_h_sideband_D0_K_PID_KLM_kaon = nullptr;

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the mu+ particle list */
    std::string m_dstListName = "";

  };
}
