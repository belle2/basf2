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
    TH1F* m_h_vx = nullptr;
    TH1F* m_h_vy = nullptr;
    TH1F* m_h_vz = nullptr;
    // ----- Detector Hits
    TH1F* m_h_npxd = nullptr;
    TH1F* m_h_nsvd = nullptr;
    TH1F* m_h_ncdc = nullptr;
    TH1F* m_h_topdig = nullptr;
    TH1F* m_h_DetPhotonARICH = nullptr;
    TH1F* m_h_klmTotalHits = nullptr;
    // ----- Resolution
    TH1F* m_h_dD0 = nullptr;
    TH1F* m_h_dZ0 = nullptr;
    TH1F* m_h_dPtcms = nullptr;
    // ----- P-value
    TH1F* m_h_Pval = nullptr;
    // ----- Beam BG condition
    TH1F* m_h_nExtraCDCHits = nullptr;
    TH1F* m_h_nECLClusters = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L3u = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L4u = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L5u = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L6u = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L3v = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L4v = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L5v = nullptr;
    TH1F* m_h_SVD_offline_occupancy_L6v = nullptr;
    // ----- Nevt/Luminosity
    TH1F* m_h_muid = nullptr;
    // ----- D*
    // ======== Other Hitograms
    TH1F* m_h_inv_p = nullptr;
    TH1F* m_h_ndf = nullptr;
    TH1F* m_h_D0 = nullptr;
    TH1F* m_h_Z0 = nullptr;
    TH1F* m_h_theta = nullptr;
    TH1F* m_h_Phi0 = nullptr;
    TH1F* m_h_Pt = nullptr;
    TH1F* m_h_Mom = nullptr;
    TH1F* m_h_klmClusterLayers = nullptr;
    TH1F* m_h_klmTotalBarrelHits = nullptr;
    TH1F* m_h_klmTotalEndcapHits = nullptr;
    TH1F* m_h_eop1 = nullptr;
    TH1F* m_h_eop2 = nullptr;
    TH1F* m_h_dPhicms = nullptr;

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the mu+ particle list */
    std::string m_muPListName = "";

  };
}
