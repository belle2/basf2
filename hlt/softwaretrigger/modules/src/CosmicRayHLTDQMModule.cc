//+
// File : CosmicRayHLTDQMModule.cc
// Description : Module to monitor Cosmic Ray on HLT
//
// Author : Chunhua LI
// Date : 19 - May - 2017
//-
#include <hlt/softwaretrigger/modules/CosmicRayHLTDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>

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
  h_z0 = new TH1F("z0", "z coordinate of the POCA", 100, -500, 500);
  h_phi0 = new TH1F("phi0", "Angle of the transverse momentum in the r-phi plane", 100, -1, 1);
  h_ncdchits = new TH1F("ncdchits", "Number of CDC hits associated to CDC track", 100, 0, 100);
  h_pValue = new TH1F("pValue", "chi2 probalility of the track fit", 100, 0, 1);
  h_ntrk = new TH1F("ntrk", "number of charged tracks", 10, 0, 10);
  h_p[0] = new TH1F("px", "track momentum in X direction", 100, -10, 10);
  h_p[1] = new TH1F("py", "track momentum in Y direction", 100, -10, 10);
  h_p[2] = new TH1F("pz", "track momentum in Z direction", 100, -10, 10);
  h_p[3] = new TH1F("p", "track momentum", 100, 0, 20);
  h_p[4] = new TH1F("pt", "transverse momentum of track", 100, 0, 20);
  h_charge = new TH1F("charge", "the charge of track", 8, -1.5, 2.5);

  //ECL
  h_ncluster = new TH1F("neclcluster", "number of ECL cluster", 30, 0, 30);
  h_e_eclcluster = new TH1F("e_ecluster", "energy of ECL cluster", 100, 0, 1.0);
  h_phi_eclcluster = new TH1F("phi_eclcluster", "phi angle of ECLCluster position", 100, -3.2, 3.2);
  h_theta_eclcluster = new TH1F("theta_eclcluster", "theta angle of ECLCluster position", 100, 0, 3.2);
  h_E1oE9_eclcluster = new TH1F("e1v8_eclcluster", "the E1/E9 energy ratio", 100, 0., 1.);
  h_Time_eclcluster = new TH1F("Time_eclcluster", "the ecl cluster time", 100, -1000., 1000.);

  //KLM
  h_nklmcluster = new TH1F("nklmcluster", "number of KLM Cluster", 10, 0, 10);
  h_pos_klmcluster[0] = new TH1F("posX", "KLMCluster's x position", 100, -100., 100);
  h_pos_klmcluster[1] = new TH1F("posY", "KLMCluster's y position", 100, -100., 100);
  h_pos_klmcluster[2] = new TH1F("posZ", "KLMCluster's z position", 100, -100., 100);
  h_innermost_klmcluster = new TH1F("innermost_klmcluster", "KLMCluster's the innermost layer with hits", 20, 0, 20);
  h_nlayer_klmcluster = new TH1F("nlayer_klmcluster", "KLMCluster's number of layers with hits", 20, 0, 20);

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

//Monitor ECL Clusters
  StoreArray<ECLCluster> eclClusters;
  if (eclClusters.isValid()) {
    h_ncluster->Fill(eclClusters.getEntries());
    for (const auto& eclCluster : eclClusters) {
      h_e_eclcluster->Fill(eclCluster.getEnergy());
      h_phi_eclcluster->Fill(eclCluster.getPhi());
      h_theta_eclcluster->Fill(eclCluster.getTheta());
      h_E1oE9_eclcluster->Fill(eclCluster.getE1oE9());
      h_Time_eclcluster->Fill(eclCluster.getTime());
    }
  }

//Monitor KLM Cluster
  StoreArray<KLMCluster> klmClusters;
  if (klmClusters.isValid()) {
    h_nklmcluster->Fill(klmClusters.getEntries());
    for (const auto& klmCluster : klmClusters) {
      h_pos_klmcluster[0]->Fill(klmCluster.getClusterPosition().x());
      h_pos_klmcluster[1]->Fill(klmCluster.getClusterPosition().y());
      h_pos_klmcluster[2]->Fill(klmCluster.getClusterPosition().z());
      h_innermost_klmcluster->Fill(klmCluster.getInnermostLayer());
      h_nlayer_klmcluster->Fill(klmCluster.getLayers());
      h_Time_klmcluster->Fill(klmCluster.getTime());
    }
  }


}
