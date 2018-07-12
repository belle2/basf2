//+
// File : CosmicRayHLTDQMModule.cc
// Description : Module to monitor Cosmic Ray on HLT
//
// Author : Chunhua LI
// Date : 19 - May - 2017
//-
#include <hlt/softwaretrigger/modules/dqm/CosmicRayHLTDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>

#include <TDirectory.h>

#include <map>
#include <string>
#include <iostream>

using namespace Belle2;
using namespace SoftwareTrigger;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CosmicRayHLTDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CosmicRayHLTDQMModule::CosmicRayHLTDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor recontruction of cosmic ray on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "Cosmic Ray DQM histograms on HLT will be put into this directory", std::string("CosmicRay_HLT"));

}

CosmicRayHLTDQMModule::~CosmicRayHLTDQMModule()
{
}

void CosmicRayHLTDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_param_histogramDirectoryName.c_str())->cd();
  //CDC
  h_d0 = new TH1F("r0", "Signed distance to the POCA in the r-phi plane", 100, -100, 100);
  h_d0->SetXTitle("r0 (cm)");
  h_z0 = new TH1F("z0", "z coordinate of the POCA", 100, -500, 500);
  h_z0->SetXTitle("z0 (cm)");
  h_phi0 = new TH1F("phi0", "Angle of the transverse momentum in the r-phi plane", 100, -1, 1);
  h_phi0->SetXTitle("#phi0 (rad.)");
  h_ncdchits = new TH1F("ncdchits", "Number of CDC hits associated to CDC track", 100, 0, 100);
  h_ncdchits->SetXTitle("#phi0 (rad.)");
  h_pValue = new TH1F("pValue", "chi2 probability of the track fit", 100, 0, 1);
  h_pValue->SetXTitle("chi2 Probability");
  h_ntrk = new TH1F("ntrk", "number of charged tracks", 10, 0, 10);
  h_ntrk->SetXTitle("Ntrk");
  h_p[0] = new TH1F("px", "track momentum in X direction", 100, -10, 10);
  h_p[0]->SetXTitle("Px (GeV)");
  h_p[1] = new TH1F("py", "track momentum in Y direction", 100, -10, 10);
  h_p[1]->SetXTitle("Py (GeV)");
  h_p[2] = new TH1F("pz", "track momentum in Z direction", 100, -10, 10);
  h_p[2]->SetXTitle("Pz (GeV)");
  h_p[3] = new TH1F("p", "track momentum", 100, 0, 20);
  h_p[3]->SetXTitle("P (GeV)");
  h_p[4] = new TH1F("pt", "transverse momentum of track", 100, 0, 20);
  h_p[4]->SetXTitle("Pt (GeV)");
  h_charge = new TH1F("charge", "the charge of track", 8, -1.5, 2.5);
  h_charge->SetXTitle("Charge");

  //ECL Clusters
  h_ncluster = new TH1F("neclcluster", "number of ECL cluster", 30, 0, 30);
  h_ncluster->SetXTitle("Number of ECL N1 Clusters");
  h_e_eclcluster = new TH1F("e_ecluster", "energy of ECL cluster", 100, 0, 1.0);
  h_e_eclcluster->SetXTitle("E (GeV)");
  h_phi_eclcluster = new TH1F("phi_eclcluster", "phi angle of ECLCluster position", 100, -3.2, 3.2);
  h_phi_eclcluster->SetXTitle("#phi (rad.)");
  h_theta_eclcluster = new TH1F("theta_eclcluster", "theta angle of ECLCluster position", 100, 0, 3.2);
  h_theta_eclcluster->SetXTitle("#theta (rad.)");
  h_E1oE9_eclcluster = new TH1F("e1v9_eclcluster", "the E1/E9 energy ratio", 100, 0., 1.);
  h_E1oE9_eclcluster->SetXTitle("E1/E9");
  h_Time_eclcluster = new TH1F("Time_eclcluster", "the ecl cluster time", 100, -1000., 1000.);
  h_Time_eclcluster->SetXTitle("Time (ns)");

  // ECL Showers
  h_nshower = new TH1F("neclshower", "number of ECL showers", 30, 0, 30);
  h_nshower->SetXTitle("Number of ECL N1 Showers");
  h_time_eclshower = new TH1F("time_eclshoer", "the ECL shower time", 100, -1000., 1000.);
  h_time_eclshower->SetXTitle("Time (ns)");
  h_e_eclshower = new TH1F("e_eshower", "energy of ECL shower", 100, 0, 1.0);
  h_e_eclshower->SetXTitle("E (GeV)");

  //KLM
  h_nbklmhit = new TH1F("nbklmhit", "number of 2D hits on barrel KLM", 30, 0, 30);
  h_nbklmhit->SetXTitle("Nhits (bKLM)");
  h_layerId_bklmhit = new TH1F("layerId_bklmhit", "layer ID of 2D hits on barrel KLM", 18, 0, 18);
  h_layerId_bklmhit->SetXTitle("Layer ID (bKLM)");
  h_sectorId_bklmhit = new TH1F("sectorId_bklmhit", "sector ID of 2D hits on barrel KLM", 10, 0, 10);
  h_sectorId_bklmhit->SetXTitle("Sector ID (bKLM)");
  h_neklmhit = new TH1F("neklmhit", "number of 2D hits on endcap KLM", 30, 0, 30);
  h_neklmhit->SetXTitle("Nhits (eKLM)");
  h_layerId_eklmhit = new TH1F("layerId_eklmhit", "layer ID of 2D hits on endcap KLM", 18, 0, 18);
  h_layerId_eklmhit->SetXTitle("Layer ID (eKLM)");
  h_sectorId_eklmhit = new TH1F("sectorId_eklmhit", "sector ID of 2D hits on endcap KLM", 10, 0, 10);
  h_sectorId_eklmhit->SetXTitle("Sector ID (eKLM)");
  oldDir->cd();
}


void CosmicRayHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void CosmicRayHLTDQMModule::event()
{

//Monitor CDC Tracks
  StoreArray<Track> tracks;
  if (tracks.isValid()) {
    h_ntrk->Fill(tracks.getEntries());
    for (const auto& track : tracks) {
      const auto* trackFit = track.getTrackFitResult(Const::muon);
      if (!trackFit) continue;
      h_d0->Fill(trackFit->getD0());
      h_z0->Fill(trackFit->getZ0());
      h_phi0->Fill(trackFit->getPhi0());
      h_ncdchits->Fill(trackFit->getHitPatternCDC().getNHits());
      h_p[0]->Fill((trackFit->getMomentum()).Px());
      h_p[1]->Fill((trackFit->getMomentum()).Py());
      h_p[2]->Fill((trackFit->getMomentum()).Pz());
      h_p[3]->Fill((trackFit->getMomentum()).Mag());
      h_p[4]->Fill((trackFit->getMomentum()).Pt());
      h_pValue->Fill(trackFit->getPValue());
      h_charge->Fill(trackFit->getChargeSign());
    }
  }

  //Monitor ECL N1 Clusters
  StoreArray<ECLCluster> eclClusters;
  int nECLClusters = 0;
  if (eclClusters.isValid()) {
    for (const auto& eclCluster : eclClusters) {
      if (eclCluster.getHypothesisId() == 5) {
        h_e_eclcluster->Fill(eclCluster.getEnergy());
        h_phi_eclcluster->Fill(eclCluster.getPhi());
        h_theta_eclcluster->Fill(eclCluster.getTheta());
        h_E1oE9_eclcluster->Fill(eclCluster.getE1oE9());
        h_Time_eclcluster->Fill(eclCluster.getTime());
        nECLClusters++;
      }
    }
    h_ncluster->Fill(nECLClusters);
  }

  // Monitor ECL N1 Showers (without timing and energy cut)
  StoreArray<ECLShower> eclShowers;
  int nECLShowers = 0;
  if (eclShowers.isValid()) {
    for (const auto& eclShower : eclShowers) {
      if (eclShower.getHypothesisId() == 5) {
        h_e_eclshower->Fill(eclShower.getEnergy());
        h_time_eclshower->Fill(eclShower.getTime());
        nECLShowers++;
      }
    }
    h_nshower->Fill(nECLShowers);
  }


//Monitor KLM
  StoreArray<BKLMHit2d> bklmhits;
  if (bklmhits.isValid()) {
    h_nbklmhit->Fill(bklmhits.getEntries());
    for (const auto& bklmhit : bklmhits) {
      h_layerId_bklmhit->Fill(bklmhit.getLayer());
      h_sectorId_bklmhit->Fill(bklmhit.getSector());
    }
  }
  /*
  StoreArray<EKLMHit2d> eklmhits;
  if (eklmhits.isValid()) {
    h_neklmhit->Fill(eklmhits.getEntries());
    for (const auto& eklmhit : eklmhits) {
      h_layerId_eklmhit->Fill(eklmhit.getLayer());
      h_sectorId_eklmhit->Fill(eklmhit.getSector());
    }
  }
  */
}
