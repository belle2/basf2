//+
// File : PhysicsTriggerDQMModule.cc
// Description : Module to monitor physics trigger on HLT
//
// Author : Chunhua LI,
// Date : 4 - March - 2015
//-
#include <skim/hlt/modules/physicstriggerDQM/PhysicsTriggerDQMModule.h>
#include <skim/hlt/dataobjects/PhysicsTriggerInformation.h>

using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhysicsTriggerDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhysicsTriggerDQMModule::PhysicsTriggerDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

}



PhysicsTriggerDQMModule::~PhysicsTriggerDQMModule()
{
}

void PhysicsTriggerDQMModule::defineHisto()
{

  h_NTrack = new TH1F("NTracks", "Number of Charged Tracks", 50, 0, 50);
  h_NTrack->SetXTitle("Number of tracks");

  h_NCluster = new TH1F("NCluster", "Number of Clusters in ECL", 500, 0, 500);
  h_NCluster->SetXTitle("Number of clusters");

  h_ESum = new TH1F("EnergySum", "Total Deposited Energy in ECL", 500, 0.0, 50.0);
  h_ESum->SetXTitle("Total deposited energy");

  h_EVis = new TH1F("EnergyVisible", "Total Visible Energy in Detectors", 500, 0.0, 50.0);
  h_EVis->SetXTitle("Total visible energy");

  h_P1 = new TH1F("P1", "The largest momentum of the charged tracks", 100, 0, 10);
  h_P1->SetXTitle("P1");

  h_E1 = new TH1F("E1", "The largest energy of the clusters in ECL ", 200, 0, 20);
  h_E1->SetXTitle("E1");

  h_MaxAngleTT = new TH1F("MaxAngleTT", "The maximum angle between the charged tracks", 100, 0, 3.5);
  h_MaxAngleTT->SetXTitle("MaxAngleTT");

  h_MaxAngleGG = new TH1F("MaxAngleGG", "The maximum angle between charged clusters", 100, 0, 3.5);
  h_MaxAngleGG->SetXTitle("MaxAngleGG");

  /*  //tracks information
    h_TrackP = new TH1F("P", "The momentum of the charged tracks", 100, 0, 10);
    h_TrackP->SetXTitle("P(Track)");

    h_TrackCostheta = new TH1F("CosthetaTrack", "The cos(theta) of the charged tracks", 100, -1, 1);
    h_TrackCostheta->SetXTitle("Costheta(Track)");


    h_TrackPhi = new TH1F("PhiTrack", "The phi angle of thecharged tracks", 200, -3.5, 3.5);
    h_TrackPhi->SetXTitle("Phi(Track)");

    h_TrackDr = new TH1F("DrTrack", "The dr of thecharged tracks", 100, 0, 10);
    h_TrackDr->SetXTitle("Dz(Track)");

    h_TrackDz = new TH1F("DzTrack", "The dz of thecharged tracks", 200, -10, 10);
    h_TrackDz->SetXTitle("Dz(Track)");

    h_ClusterE = new TH1F("E", "The energy of the clusters", 100, 0, 10);
    h_ClusterE->SetXTitle("ECluster");

    h_ClusterTheta = new TH1F("ThetaCluster", "The theta angle of the clusters", 100, 0, 3.5);
    h_ClusterTheta->SetXTitle("Theta(Cluster)");

    //h_ClusterPhi = new TH1F("phi","The phi angle of the clusters",100,0,10);
    //h_ClusterPhi->SetXTitle("#phi(Cluster)");

    h_ClusterTiming = new TH1F("Timing", "The timing of the clusters", 500, 0, 3500);
    h_ClusterTiming->SetXTitle("Timing(Cluster)");
  */

}


void PhysicsTriggerDQMModule::initialize()
{
  REG_HISTOGRAM
}


void PhysicsTriggerDQMModule::beginRun()
{
  B2INFO("beginRun called.");
}




void PhysicsTriggerDQMModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void PhysicsTriggerDQMModule::terminate()
{
  B2INFO("terminate called");
}



//
// User defined functions
//


void PhysicsTriggerDQMModule::event()
{
  StoreArray<PhysicsTriggerInformation> phytriInfos;
  if (!phytriInfos.getEntries()) {
    phytriInfos.appendNew(PhysicsTriggerInformation());
    B2DEBUG(200, "No entry in PhysicsTriggerInformation");
  }
  PhysicsTriggerInformation* phytriInfo = phytriInfos[0];
  h_NTrack->Fill(phytriInfo->getNGoodTrack());
  h_NCluster->Fill(phytriInfo->getNGoodECLClusters());
  h_ESum->Fill(phytriInfo->getEsum());
  h_EVis->Fill(phytriInfo->getEvis());
  h_P1->Fill(phytriInfo->getP1());
  h_E1->Fill(phytriInfo->getE1());
  h_MaxAngleTT->Fill(phytriInfo->getAngleTT());
  h_MaxAngleGG->Fill(phytriInfo->getAngleGG());
  /*
    vector<float> trackP = phytriInfo->getP();
    vector<float> trackCostheta = phytriInfo->getCosTheta();
    vector<float> trackPhi = phytriInfo->getPhi();
    vector<float> trackDr = phytriInfo->getDr();
    vector<float> trackDz = phytriInfo->getDz();
    for (unsigned int i = 0; i < trackP.size(); i++) {
      h_TrackP->Fill(trackP[i]);
      h_TrackCostheta->Fill(trackCostheta[i]);
      h_TrackPhi->Fill(trackPhi[i]);
      h_TrackDr->Fill(trackDr[i]);
      h_TrackDz->Fill(trackDz[i]);
    }

    vector<float>clusterE = phytriInfo->getECLClusterE();
    vector<float>clusterTheta = phytriInfo->getECLClusterTheta();
    //vector<float>clusterPhi = phytriInfo->getECLClusterPhi();
    vector<float>clusterTiming = phytriInfo->getECLClusterTiming();
    for (unsigned int i = 0; i < clusterE.size(); i++) {
      h_ClusterE->Fill(clusterE[i]);
      h_ClusterTheta->Fill(clusterTheta[i]);
  //  h_ClusterPhi->Fill(clusterPhi[i]);
      h_ClusterTiming->Fill(clusterTiming[i]);
    }
  */
}
