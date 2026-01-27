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
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class PhysicsObjectsMiraBelleTrgEfficiencyModule : public HistoModule {
  public:

    /**
     * Constructor.
     */
    PhysicsObjectsMiraBelleTrgEfficiencyModule();

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
    /** Trigger identifier strings used to select events for the histograms */
    std::string m_triggerIdentifierTau2trk = "";
    std::string m_triggerIdentifierTauNtrk = "";

    // ======== Hitograms for run dependence
    // ----- Efficiency of L1 bits
    TH1F* m_h_L1ECL1x1 = nullptr;/**< histogram for efficiency of ECL TRG bits using 1x1 prong tau pairs */
    TH1F* m_h_L1ECL1x3 = nullptr;/**< histogram for efficiency of ECL TRG bits using 1x3 prong tau pairs */
    TH1F* m_h_L1CDC1x1 = nullptr;/**< histogram for efficiency of CDC TRG bits using 1x1 prong tau pairs */
    TH1F* m_h_L1CDC1x3 = nullptr;/**< histogram for efficiency of CDC TRG bits using 1x3 prong tau pairs */
    TH1F* m_h_L1CDCKLM1x1 = nullptr;/**< histogram for efficiency of CDC-KLM TRG bits using 1x1 prong tau pairs */
    TH1F* m_h_L1CDCKLM1x3 = nullptr;/**< histogram for efficiency of CDC-KLM TRG bits using 1x3 prong tau pairs */

    /** Array of ECL trigger bits */
    std::array<std::string, 14> m_ECLTrgBit =
    {"hie", "c4", "eclmumu", "lml0", "lml1", "lml2", "lml4", "lml6", "lml7", "lml8", "lml9", "lml10", "lml12", "lml13"};

    /** Array of CDC trigger bits */
    std::array<std::string, 17> m_CDCTrgBit =
    {"ff30", "ffo", "fff", "fs30", "ss30", "fso", "sso", "ffs", "fss", "sss", "fy30", "fyo", "syo", "ffy", "fyy", "yyy", "y"};

    /** Array of KLM and CDC-KLM bits */
    std::array<std::string, 6> m_CDCKLMTrgBit =
    {"mu_b2b", "mu_eb2b", "cdcklm1", "cdcklm2", "seklm1", "seklm2"};

    /** Name of the tau+tau- (1x1) particle list */
    std::string m_tautau1x1PListName = "";

    /** Name of the tau+tau- (1x3) particle list */
    std::string m_tautau1x3PListName = "";

    /** Reference bits for L1 Trigger efficiency monitor */
    bool m_ECL_ref = false; /** Flag for ECL bit */
    bool m_CDC_ref = false; /** Flag for CDC bit */



  };
}
