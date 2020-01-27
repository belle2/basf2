/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa                                         *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDDQMClustersOnTrackModule.h"

#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace SoftwareTrigger;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMClustersOnTrack)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMClustersOnTrackModule::SVDDQMClustersOnTrackModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module for Clusters related to Tracks.");

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("skipHLTRejectedEvents", m_skipRejectedEvents, "If TRUE skip events rejected by HLT", bool(true));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("SVDClsTrk"));

  m_histoList = new TList();
}


SVDDQMClustersOnTrackModule::~SVDDQMClustersOnTrackModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMClustersOnTrackModule::defineHisto()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2FATAL("Missing geometry for VXD, check steering file.");
  }
  if (gTools->getNumberOfSVDLayers() == 0) {
    B2WARNING("Missing geometry for SVD, SVD-DQM is skiped.");
    return;
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  int ChargeBins = 80;
  float ChargeMax = 80;
  int SNRBins = 50;
  float SNRMax = 100;
  int TimeBins = 50;
  float TimeMin = -100;
  float TimeMax = 100;

  int MaxBinBins = 6;
  int MaxBinMax = 6;

  //----------------------------------------------------------------
  // Charge of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  string name = str(format("SVDTRK_ClusterChargeU3"));
  string title = str(format("SVD U-Cluster-on-Track Charge for layer 3 sensors"));
  m_clsTrkChargeU3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeU3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeU3);
  name = str(format("SVDTRK_ClusterChargeV3"));
  title = str(format("SVD V-Cluster-on-Track Charge for layer 3 sensors"));
  m_clsTrkChargeV3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeV3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeV3);

  name = str(format("SVDTRK_ClusterChargeU456"));
  title = str(format("SVD U-Cluster-on-Track Charge for layers 4,5,6 sensors"));
  m_clsTrkChargeU456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeU456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeU456);

  name = str(format("SVDTRK_ClusterChargeV456"));
  title = str(format("SVD V-Cluster-on-Track Charge for layers 4,5,6 sensors"));
  m_clsTrkChargeV456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeV456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeV456);

  //----------------------------------------------------------------
  // SNR of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDTRK_ClusterSNRU3"));
  title = str(format("SVD U-Cluster-on-Track SNR for layer 3 sensors"));
  m_clsTrkSNRU3 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clsTrkSNRU3->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRU3);
  name = str(format("SVDTRK_ClusterSNRV3"));
  title = str(format("SVD V-Cluster-on-Track SNR for layer 3 sensors"));
  m_clsTrkSNRV3 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clsTrkSNRV3->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRV3);

  name = str(format("SVDTRK_ClusterSNRU456"));
  title = str(format("SVD U-Cluster-on-Track SNR for layers 4,5,6 sensors"));
  m_clsTrkSNRU456 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clsTrkSNRU456->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRU456);
  name = str(format("SVDTRK_ClusterSNRV456"));
  title = str(format("SVD V-Cluster-on-Track SNR for layers 4,5,6 sensors"));
  m_clsTrkSNRV456 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clsTrkSNRV456->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRV456);

  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDTRK_ClusterTimeU3"));
  title = str(format("SVD U-Cluster-on-Track Time for layer 3 sensors"));
  m_clsTrkTimeU3 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeU3->GetXaxis()->SetTitle("clusters time [ns]");
  m_clsTrkTimeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeU3);
  name = str(format("SVDTRK_ClusterTimeV3"));
  title = str(format("SVD V-Cluster-on-Track Time for layer 3 sensors"));
  m_clsTrkTimeV3 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeV3->GetXaxis()->SetTitle("cluster time [ns]");
  m_clsTrkTimeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeV3);

  name = str(format("SVDTRK_ClusterTimeU456"));
  title = str(format("SVD U-Cluster-on-Track Time for layers 4,5,6 sensors"));
  m_clsTrkTimeU456 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeU456->GetXaxis()->SetTitle("cluster time [ns]");
  m_clsTrkTimeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeU456);
  name = str(format("SVDTRK_ClusterTimeV456"));
  title = str(format("SVD V-Cluster-on-Track Time for layers 4,5,6 sensors"));
  m_clsTrkTimeV456 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeV456->GetXaxis()->SetTitle("cluster time [ns]");
  m_clsTrkTimeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeV456);

  //----------------------------------------------------------------
  // MaxBin of strips for all sensors (offline ZS)
  //----------------------------------------------------------------
  name = str(format("SVDTRK_StripMaxBinUAll"));
  title = str(format("SVD U-Strip-on-Track MaxBin for all sensors"));
  m_stripMaxBinUAll = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinUAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinUAll);
  name = str(format("SVDTRK_StripMaxBinVAll"));
  title = str(format("SVD V-Strip-on-Track MaxBin for all sensors"));
  m_stripMaxBinVAll = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinVAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinVAll);


  oldDir->cd();
}


void SVDDQMClustersOnTrackModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() != 0) {

    StoreArray<Track> storeTracks;

    storeTracks.isOptional();

  }
}

void SVDDQMClustersOnTrackModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  m_expNumber = evtMetaData->getExperiment();
  m_runNumber = evtMetaData->getRun();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

  // Add experiment and run number to the title of selected histograms (CR shifter plots)
  TString runID = TString::Format(" ~ Exp%d Run%d", m_expNumber, m_runNumber);
  TObject* obj;
  TIter nextH(m_histoList);
  while ((obj = nextH()))
    if (obj->InheritsFrom("TH1")) {
      ((TH1F*)obj)->SetTitle(obj->GetTitle() + runID);
      if (obj != NULL)((TH1F*)obj)->Reset();
    }

}

void SVDDQMClustersOnTrackModule::event()
{

  //check HLT decision and increase number of events only if the event has been accepted

  if (m_skipRejectedEvents && (m_resultStoreObjectPointer.isValid())) {
    const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
    if (!eventAccepted) return;
  }

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

  StoreArray<Track> storeTracks;

  if (!storeTracks || !storeTracks.getEntries()) {
    return;
  }

  BOOST_FOREACH(Track & track, storeTracks) {

    const TrackFitResult* tfr = track.getTrackFitResult(Const::pion);
    if (!tfr) continue;

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);

    for (int cl = 0 ; cl < (int)svdClustersTrack.size(); cl++) {

      int iLayer = svdClustersTrack[cl]->getSensorID().getLayerNumber();
      if (svdClustersTrack[cl]->isUCluster()) {

        if (iLayer == 3) {
          if (m_clsTrkChargeU3 != NULL) m_clsTrkChargeU3->Fill(svdClustersTrack[cl]->getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRU3 != NULL) m_clsTrkSNRU3->Fill(svdClustersTrack[cl]->getSNR());
          if (m_clsTrkTimeU3 != NULL) m_clsTrkTimeU3->Fill(svdClustersTrack[cl]->getClsTime());
        } else {
          if (m_clsTrkChargeU456 != NULL) m_clsTrkChargeU456->Fill(svdClustersTrack[cl]->getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRU456 != NULL) m_clsTrkSNRU456->Fill(svdClustersTrack[cl]->getSNR());
          if (m_clsTrkTimeU456 != NULL) m_clsTrkTimeU456->Fill(svdClustersTrack[cl]->getClsTime());
        }

        RelationVector<SVDRecoDigit> recoDigits = svdClustersTrack[cl]->getRelationsTo<SVDRecoDigit>();
        if (recoDigits.size() == 0) continue;

        for (UInt_t re = 0; re < recoDigits.size(); re++) {

          RelationVector<SVDShaperDigit> shaper = recoDigits[re]->getRelationsTo<SVDShaperDigit>();
          if (m_stripMaxBinUAll != NULL) m_stripMaxBinUAll->Fill(shaper[0]->getMaxTimeBin());
        }


      } else {


        if (iLayer == 3) {
          if (m_clsTrkChargeV3 != NULL) m_clsTrkChargeV3->Fill(svdClustersTrack[cl]->getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRV3 != NULL) m_clsTrkSNRV3->Fill(svdClustersTrack[cl]->getSNR());
          if (m_clsTrkTimeV3 != NULL) m_clsTrkTimeV3->Fill(svdClustersTrack[cl]->getClsTime());
        } else {
          if (m_clsTrkChargeV456 != NULL) m_clsTrkChargeV456->Fill(svdClustersTrack[cl]->getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRV456 != NULL) m_clsTrkSNRV456->Fill(svdClustersTrack[cl]->getSNR());
          if (m_clsTrkTimeV456 != NULL) m_clsTrkTimeV456->Fill(svdClustersTrack[cl]->getClsTime());
        }

        RelationVector<SVDRecoDigit> recoDigits = svdClustersTrack[cl]->getRelationsTo<SVDRecoDigit>();
        if (recoDigits.size() == 0) continue;

        for (UInt_t re = 0; re < recoDigits.size(); re++) {

          RelationVector<SVDShaperDigit> shaper = recoDigits[re]->getRelationsTo<SVDShaperDigit>();
          if (m_stripMaxBinVAll != NULL) m_stripMaxBinVAll->Fill(shaper[0]->getMaxTimeBin());

        }

      }

    }
  }
}


void SVDDQMClustersOnTrackModule::terminate()
{

  delete m_histoList;

}