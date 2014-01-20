#include "testbeam/vxd/modules/DQM/VXDTFDQMModule.h"
#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <genfit/TrackCand.h>

#include "TVector3.h"
#include "TVectorD.h"

#include "TDirectory.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDTFDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDTFDQMModule::VXDTFDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("VXDTF DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("GFTrackCandidatesColName", m_storeTrackCandsName,
           "Name of collection holding the genfit::TrackCandidates", std::string("caTracks"));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Track fit Validation histograms will be put into this directory", std::string("vxdtf"));
}


VXDTFDQMModule::~VXDTFDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VXDTFDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
//  oldDir->cd();
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();
//  gStyle->SetOptStat(111111); // includes extra info like underflow and overflow into the legend of the histogram

  m_histoMomentum = new TH1F("VXDTFMomentum", "VXDTF: Total momentum estimated", 100, 0, 50);
  m_histoMomentum->GetXaxis()->SetTitle("value of estimated momentum");
  m_histoMomentum->GetYaxis()->SetTitle("count");

  m_histoPT = new TH1F("VXDTFMomentumT", "VXDTF: Transverse momentum estimated", 100, 0, 50);
  m_histoPT->GetXaxis()->SetTitle("value of estimated transverse momentum");
  m_histoPT->GetYaxis()->SetTitle("count");

  m_histoMomentumX = new TH1F("VXDTFMomentumX", "VXDTF: Momentum in X estimated", 150, -30, 30);
  m_histoMomentumX->GetXaxis()->SetTitle("value of estimated momentum in X");
  m_histoMomentumX->GetYaxis()->SetTitle("count");

  m_histoMomentumY = new TH1F("VXDTFMomentumY", "VXDTF: Momentum in Y  estimated", 150, -3, 3);
  m_histoMomentumY->GetXaxis()->SetTitle("value of estimated momentum in Y");
  m_histoMomentumY->GetYaxis()->SetTitle("count");

  m_histoMomentumZ = new TH1F("VXDTFMomentumZ", "VXDTF: Momentum in Z  estimated", 150, -3, 3);
  m_histoMomentumZ->GetXaxis()->SetTitle("value of estimated momentum in Z");
  m_histoMomentumZ->GetYaxis()->SetTitle("count");

  m_histoNumHitsUsed = new TH1I("VXDTFNHits", "VXDTF: Num of hits used for TC", 20, -0.5, 18.5);
  m_histoNumHitsUsed->GetXaxis()->SetTitle("Num of Hits used for TC");
  m_histoNumHitsUsed->GetYaxis()->SetTitle("count");

  m_histoNumHitsIgnored = new TH1I("VXDTFNHitDifference", "VXDTF: Total num of Hits - num of hits used for TC", 20, -0.5, 18.5);
  m_histoNumHitsIgnored->GetXaxis()->SetTitle("Difference in num of hits");
  m_histoNumHitsIgnored->GetYaxis()->SetTitle("count");

  m_histoNumTCsPerEvent = new TH1I("VXDTFNTCs", "VXDTF: Total num of TCs per event", 20, -0.51, 18.5);
  m_histoNumHitsIgnored->GetXaxis()->SetTitle("num of TCs per event");
  m_histoNumHitsIgnored->GetYaxis()->SetTitle("count");


  /// WARNING TODO:
  /**
   * 2D-Hist: vorhandene Cluster gegen benützte Cluster
   *  - " - nur für jeden layer einzeln...
   * */
  oldDir->cd();
}


void VXDTFDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDCluster>::required();
//   StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<genfit::TrackCand>::required(m_storeTrackCandsName);
//   StoreArray<genfit::TrackCand> caTrackCandidates(m_storeTrackCandsName);

  //Store names to speed up creation later
//   m_storeClustersName = storeClusters.getName();
//   m_storeTrackCandsName = caTrackCandidates.getName();

}

void VXDTFDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_histoMomentum->Reset();
  m_histoPT->Reset();
  m_histoMomentumX->Reset();
  m_histoMomentumY->Reset();
  m_histoMomentumZ->Reset();
  m_histoNumHitsUsed->Reset();
  m_histoNumHitsIgnored->Reset();
  m_histoNumTCsPerEvent->Reset();
}


void VXDTFDQMModule::event()
{
  const StoreArray<genfit::TrackCand> caTrackCandidates(m_storeTrackCandsName);
  const StoreArray<SVDCluster> storeClusters(m_storeClustersName);


  // collecting info from TCs (estimated momentum, nHits, ..)
  int nCaTCs = caTrackCandidates.getEntries();
  int nSVDClusters = storeClusters.getEntries();
  TVector3 momentum;
  for (const auto aTrackCand : caTrackCandidates) {
    int nHits = aTrackCand.getNHits();
    TVectorD stateSeed = aTrackCand.getStateSeed();
    momentum[0] = stateSeed(3); momentum[1] = stateSeed(4); momentum[2] = stateSeed(5);
    double fullMomentum = momentum.Mag();
    double traverseMomentum = momentum.Perp();
    double momentumX = momentum.X();
    double momentumY = momentum.Y();
    double momentumZ = momentum.Z();

    m_histoMomentum->Fill(fullMomentum);
    m_histoPT->Fill(traverseMomentum);
    m_histoMomentumX->Fill(momentumX);
    m_histoMomentumY->Fill(momentumY);
    m_histoMomentumZ->Fill(momentumZ);
    m_histoNumHitsUsed->Fill(nHits);
    m_histoNumHitsIgnored->Fill(nSVDClusters - nHits);
  }
  if (nCaTCs == 0) { // in this case, the for-loop was not executed
    m_histoNumHitsUsed->Fill(0);
    m_histoNumHitsIgnored->Fill(nSVDClusters);
  }
  m_histoNumTCsPerEvent->Fill(nCaTCs);

}


void VXDTFDQMModule::endRun()
{
}


void VXDTFDQMModule::terminate()
{
}
