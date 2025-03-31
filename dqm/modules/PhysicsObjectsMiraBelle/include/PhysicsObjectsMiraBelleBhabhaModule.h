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
  class PhysicsObjectsMiraBelleBhabhaModule : public HistoModule {
  public:

    /**
     * Constructor.
     */
    PhysicsObjectsMiraBelleBhabhaModule();

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
    // ======== Hitograms for run dependence
    // ----- Detector Hits
    TH1F* m_h_npxd = nullptr;/**< histogram for number of PXD hits associated with electron track */
    TH1F* m_h_nsvd = nullptr;/**< histogram for number of SVD hits associated with electron track */
    TH1F* m_h_ncdc = nullptr;/**< histogram for number of CDC hits associated with electron track */
    TH1F* m_h_topdig = nullptr;/**< histogram for TOP digits associated with electron track */
    TH1F* m_h_DetPhotonARICH = nullptr;/**< histogram for number of photon in ARICH associated with electron track */
    // ----- Resolution
    TH1F* m_h_dD0 = nullptr;/**< histogram for D0 difference between e+ and e-*/
    TH1F* m_h_dZ0 = nullptr;/**< histogram for Z0 difference between e+ and e-*/
    TH1F* m_h_dPtcms = nullptr;/**< histogram for Pt difference between e+ and e-*/
    // ----- P-value
    TH1F* m_h_Pval = nullptr;/**< histogram for Pvalue of tracks in CDC*/
    // ----- Beam BG condition
    TH1F* m_h_nExtraCDCHits = nullptr;/**< histogram for number of CDC hits not associated with any tracks */
    TH1F* m_h_nECLClusters = nullptr;/**< histogram for ECL clusters */
    // ----- Nevt/Luminosity
    TH1F* m_h_electronid = nullptr;/**< histogram for electron ID */
    // ======== Other Hitograms
    TH1F* m_h_inv_p = nullptr;/**< histogram for invariant mass of di-electron */
    TH1F* m_h_ndf = nullptr;/**< histogram for n.d.f. of electron track */
    TH1F* m_h_D0 = nullptr;/**< histogram for D0 of electron track */
    TH1F* m_h_Z0 = nullptr;/**< histogram for Z0 of electron track */
    TH1F* m_h_theta = nullptr;/**< histogram for theta of electron track */
    TH1F* m_h_theta_lab = nullptr;/**< histogram for theta of electron track in lab frame */
    TH1F* m_h_Phi0 = nullptr;/**< histogram for phi of electron track */
    TH1F* m_h_Pt = nullptr;/**< histogram for Pt of electron track */
    TH1F* m_h_Mom = nullptr;/**< histogram for Momentum of electron track */
    TH1F* m_h_dPhicms = nullptr;/**< histogram for phi difference between e+ and e- in CMS */
    TH1F* m_h_dThetacms = nullptr;/**< histogram for theta difference between e+ and e- in CMS */

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the e+ particle list */
    std::string m_ePListName = "";

    /** Name of the e+e- (Upsilon) particle list */
    std::string m_bhabhaPListName = "";

  };
}
