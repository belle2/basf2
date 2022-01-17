/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  class PhysicsObjectsMiraBelleModule : public HistoModule {
  public:
    /** Constructor, for setting module description and parameters. */
    PhysicsObjectsMiraBelleModule();

    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    void defineHisto() override;

  private:
    // ======== Hitograms for run dependence
    // ----- IP position
    /* TH1F* m_h_vx = nullptr;/\**< histogram for x coordiate of IP position *\/ */
    /* TH1F* m_h_vy = nullptr;/\**< histogram for y coordiate of IP position *\/ */
    /* TH1F* m_h_vz = nullptr;/\**< histogram for z coordiate of IP position *\/ */
    // ----- Detector Hits
    TH1F* m_h_npxd = nullptr;/**< histogram for number of PXD hits associated with muon track */
    TH1F* m_h_nsvd = nullptr;/**< histogram for number of SVD hits associated with muon track */
    TH1F* m_h_ncdc = nullptr;/**< histogram for number of CDC hits associated with muon track */
    TH1F* m_h_topdig = nullptr;/**< histogram for TOP digits associated with muon track */
    TH1F* m_h_DetPhotonARICH = nullptr;/**< histogram for number of photon in ARICH associated with muon track */
    TH1F* m_h_klmTotalHits = nullptr;/**< histogram for number of KLM hits associated with muon track */
    TH1F* m_h_klmTotalBarrelHits = nullptr;/**< histogram for number of BKLM hits associated with muon track */
    TH1F* m_h_klmTotalEndcapHits = nullptr;/**< histogram for number of EKLM hits associated with muon track */
    TH1F* m_h_klmClusterLayers = nullptr;/**< histogram for number of KLM layers with cluster associated with muon track */
    // ----- Resolution
    TH1F* m_h_dD0 = nullptr;/**< histogram for D0 difference between mu+ and mu-*/
    TH1F* m_h_dZ0 = nullptr;/**< histogram for Z0 difference between mu+ and mu-*/
    TH1F* m_h_dPtcms = nullptr;/**< histogram for Pt difference between mu+ and mu-*/
    // ----- P-value
    TH1F* m_h_Pval = nullptr;/**< histogram for Pvalue of tracks in CDC*/
    // ----- Beam BG condition
    TH1F* m_h_nExtraCDCHits = nullptr;/**< histogram for number of CDC hits not associated with any tracks */
    TH1F* m_h_nECLClusters = nullptr;/**< histogram for ECL clusters */
    // ----- Nevt/Luminosity
    TH1F* m_h_muid = nullptr;/**< histogram for muon ID */
    // ----- D*
    // ======== Other Hitograms
    TH1F* m_h_inv_p = nullptr;/**< histogram for invariant mass of di-muon */
    TH1F* m_h_ndf = nullptr;/**< histogram for n.d.f. of muon track */
    TH1F* m_h_D0 = nullptr;/**< histogram for D0 of muon track */
    TH1F* m_h_Z0 = nullptr;/**< histogram for Z0 of muon track */
    TH1F* m_h_theta = nullptr;/**< histogram for theta of muon track */
    TH1F* m_h_Phi0 = nullptr;/**< histogram for phi of muon track */
    TH1F* m_h_Pt = nullptr;/**< histogram for Pt of muon track */
    TH1F* m_h_Mom = nullptr;/**< histogram for Momentum of muon track */
    TH1F* m_h_dPhicms = nullptr;/**< histogram for phi difference between mu+ and mu- in CMS */

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the mu+ particle list */
    std::string m_muPListName = "";

    /** Name of the mu+mu- (Upsilon) particle list */
    std::string m_mumuPListName = "";

  };
}
