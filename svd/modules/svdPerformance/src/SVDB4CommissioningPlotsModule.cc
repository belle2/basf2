/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svdPerformance/SVDB4CommissioningPlotsModule.h>
#include <framework/datastore/RelationVector.h>
#include <time.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDB4CommissioningPlots)

SVDB4CommissioningPlotsModule::SVDB4CommissioningPlotsModule() : Module()
  , m_nTracks(0), m_Pvalue(), m_mom(0), m_nSVDhits(0)
{

  setDescription("This module check performances of SVD reconstruction on Commissioning data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDB4CommissioningPlots_output.root"));

  addParam("RecoDigitsName", m_RecoDigitName, "Name of RecoDigit Store Array.", std::string("SVDRecoDigits"));
  addParam("ClustersName", m_ClusterName, "Name of Cluster Store Array.", std::string("SVDClusters"));
  addParam("TrackListName", m_TrackName, "Name of Track Store Array.", std::string("Tracks"));
  addParam("TrackFitResultListName", m_TrackFitResultName, "Name of TracksFitResult Store Array.", std::string("TrackFitResults"));
}

SVDB4CommissioningPlotsModule::~SVDB4CommissioningPlotsModule()
{

}

void SVDB4CommissioningPlotsModule::initialize()
{

  m_svdRecos.isOptional(m_RecoDigitName);
  m_svdClusters.isRequired(m_ClusterName);
  m_Tracks.isOptional(m_TrackName);
  m_recoTracks.isOptional();
  m_tfr.isOptional(m_TrackFitResultName);


  B2INFO("      RecoDigits: " << m_RecoDigitName);
  B2INFO("        Clusters: " << m_ClusterName);
  B2INFO("          Tracks: " << m_TrackName);
  B2INFO(" TrackFitResults: " << m_TrackFitResultName);



  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  m_ntracks = 0;
}

void SVDB4CommissioningPlotsModule::beginRun()
{
  m_nEvents = 0;

  //RECO DIGITS
  TH1F hRecoCharge("reco_charge_L@layerL@ladderS@sensor@view",
                   "Charge of RecoDigits in @layer.@ladder.@sensor @view/@side side",
                   100, 0 , 1000);
  hRecoCharge.GetXaxis()->SetTitle("charge (ke-)");
  h_recoCharge = new SVDHistograms<TH1F>(hRecoCharge);

  TH1F hRecoEnergy("reco_energy_L@layerL@ladderS@sensor@view",
                   "Energy of RecoDigits in @layer.@ladder.@sensor @view/@side side",
                   100, 0 , 360);
  hRecoEnergy.GetXaxis()->SetTitle("energy (keV)");
  h_recoEnergy = new SVDHistograms<TH1F>(hRecoEnergy);

  TH1F hRecoTime("reco_time_L@layerL@ladderS@sensor@view",
                 "Time of RecoDigits in @layer.@ladder.@sensor @view/@side side",
                 200, -100 , 100);
  hRecoTime.GetXaxis()->SetTitle("time (ns)");
  h_recoTime = new SVDHistograms<TH1F>(hRecoTime);

  TH1F hRecoNoise("reco_noise_L@layerL@ladderS@sensor@view",
                  "Noise of RecoDigits in @layer.@ladder.@sensor @view/@side side",
                  200, 300 , 1800);
  hRecoNoise.GetXaxis()->SetTitle("strip noise (e-)");
  h_recoNoise = new SVDHistograms<TH1F>(hRecoNoise);

  //CLUSTER NOT RELATED TO TRACKS
  TH1F hClusterCharge("cluster_charge_L@layerL@ladderS@sensor@view",
                      "Charge of Clusters in @layer.@ladder.@sensor @view/@side side",
                      100, 0 , 100);
  hClusterCharge.GetXaxis()->SetTitle("charge (ke-)");
  h_clusterCharge = new SVDHistograms<TH1F>(hClusterCharge);

  TH1F hClusterSize("cluster_size_L@layerL@ladderS@sensor@view",
                    "Clusters Size for @layer.@ladder.@sensor @view/@side side",
                    20, 0 , 20);
  hClusterSize.GetXaxis()->SetTitle("cluster size");
  h_clusterSize = new SVDHistograms<TH1F>(hClusterSize);

  TH1F hClusterSNR("cluster_SNR_L@layerL@ladderS@sensor@view",
                   "SNR of Clusters in @layer.@ladder.@sensor @view/@side side",
                   100, 0 , 140);
  hClusterSNR.GetXaxis()->SetTitle("SNR");
  h_clusterSNR = new SVDHistograms<TH1F>(hClusterSNR);

  TH1F hClusterEnergy("cluster_energy_L@layerL@ladderS@sensor@view",
                      "Energy of Clusters in @layer.@ladder.@sensor @view/@side side",
                      100, 0 , 360);
  hClusterEnergy.GetXaxis()->SetTitle("energy (keV)");
  h_clusterEnergy = new SVDHistograms<TH1F>(hClusterEnergy);

  TH1F hClusterTime("cluster_time_L@layerL@ladderS@sensor@view",
                    "Time of Clusters in @layer.@ladder.@sensor @view/@side side",
                    200, -100 , 100);
  hClusterTime.GetXaxis()->SetTitle("time (ns)");
  h_clusterTime = new SVDHistograms<TH1F>(hClusterTime);

  //CLUSTER RELATED TO TRACKS
  TH1F hClusterTrkCharge("clusterTrk_charge_L@layerL@ladderS@sensor@view",
                         "Charge of Clusters Related to Tracks in @layer.@ladder.@sensor @view/@side side",
                         100, 0 , 100);
  hClusterTrkCharge.GetXaxis()->SetTitle("charge (ke-)");
  h_clusterTrkCharge = new SVDHistograms<TH1F>(hClusterTrkCharge);

  TH1F hClusterTrkSize("clusterTrk_size_L@layerL@ladderS@sensor@view",
                       "Cluster Size for @layer.@ladder.@sensor @view/@side side",
                       20, 0 , 20);
  hClusterTrkSize.GetXaxis()->SetTitle("cluster size");
  h_clusterTrkSize = new SVDHistograms<TH1F>(hClusterTrkSize);

  TH1F hClusterTrkSNR("clusterTrk_SNR_L@layerL@ladderS@sensor@view",
                      "SNR of Clusters Related to Tracks in @layer.@ladder.@sensor @view/@side side",
                      100, 0 , 140);
  hClusterTrkSNR.GetXaxis()->SetTitle("SNR");
  h_clusterTrkSNR = new SVDHistograms<TH1F>(hClusterTrkSNR);

  TH1F hClusterTrkEnergy("clusterTrk_energy_L@layerL@ladderS@sensor@view",
                         "Energy of Clusters Related to Tracks in @layer.@ladder.@sensor @view/@side side",
                         100, 0 , 360);
  hClusterTrkEnergy.GetXaxis()->SetTitle("energy (keV)");
  h_clusterTrkEnergy = new SVDHistograms<TH1F>(hClusterTrkEnergy);

  TH1F hClusterTrkTime("clusterTrk_time_L@layerL@ladderS@sensor@view",
                       "Time of Clusters Related to Tracks in @layer.@ladder.@sensor @view/@side side",
                       200, -100 , 100);
  hClusterTrkTime.GetXaxis()->SetTitle("time (ns)");
  h_clusterTrkTime = new SVDHistograms<TH1F>(hClusterTrkTime);

  TH1F hClusterTrkInterstripPos("clusterTrk_interstripPos_L@layerL@ladderS@sensor@view",
                                "Interstrip Position of Clusters Related to Tracks in @layer.@ladder.@sensor @view/@side side",
                                400, 0 , 1);
  hClusterTrkInterstripPos.GetXaxis()->SetTitle("interstrip position");
  h_clusterTrkInterstripPos = new SVDHistograms<TH1F>(hClusterTrkInterstripPos);

  //tracks
  m_nTracks = new TH1F("h1nTracks", "number of Tracks per event", 50, 0, 50);
  m_nTracks->GetXaxis()->SetTitle("n Tracks");
  m_Pvalue = new TH1F("h1pValue", "Tracks p value", 100, 0, 1);
  m_Pvalue->GetXaxis()->SetTitle("p value");
  m_mom = new TH1F("h1momentum", " Tracks Momentum", 200, 0, 10);
  m_mom->GetXaxis()->SetTitle("p (GeV/c)");
  m_nSVDhits = new TH1F("h1nSVDhits", "# SVD hits per track", 20, 0, 20);
  m_nSVDhits->GetXaxis()->SetTitle("# SVD hits");


}

void SVDB4CommissioningPlotsModule::event()
{

  //  StoreObjPtr<EventMetaData> eventMD;

  m_nEvents++;
  float c_eTOkeV = 3.6 / 1000; //keV = e * c_eTOkeV

  //tracks
  if (m_Tracks) {
    m_nTracks->Fill(m_Tracks.getEntries());
    m_ntracks += m_Tracks.getEntries();
  }
  BOOST_FOREACH(Track & track, m_Tracks) {

    const TrackFitResult* tfr = track.getTrackFitResultWithClosestMass(Const::pion);
    if (tfr) {
      m_Pvalue->Fill(tfr->getPValue());
      m_mom->Fill(tfr->getMomentum().Mag());
      m_nSVDhits->Fill((tfr->getHitPatternVXD()).getNSVDHits());
    }

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);

    for (int cl = 0 ; cl < (int)svdClustersTrack.size(); cl++) {

      float clCharge = svdClustersTrack[cl]->getCharge();
      float clEnergy = svdClustersTrack[cl]->getCharge() * c_eTOkeV;
      int clSize = svdClustersTrack[cl]->getSize();
      float clSN = svdClustersTrack[cl]->getSNR();
      float clTime = svdClustersTrack[cl]->getClsTime();
      float clPosition = svdClustersTrack[cl]->getPosition();
      VxdID::baseType theVxdID = (VxdID::baseType)svdClustersTrack[cl]->getSensorID();
      int side = svdClustersTrack[cl]->isUCluster();

      const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(theVxdID);
      float pitch = aSensorInfo->getVPitch();
      if (side == 1)
        pitch = aSensorInfo->getUPitch();
      float clInterstripPos = fmod(clPosition, pitch) / pitch;

      h_clusterTrkCharge->fill(theVxdID, side, clCharge / 1000.);
      h_clusterTrkSize->fill(theVxdID, side, clSize);
      h_clusterTrkSNR->fill(theVxdID, side, clSN);
      h_clusterTrkEnergy->fill(theVxdID, side, clEnergy);
      h_clusterTrkTime->fill(theVxdID, side, clTime);
      h_clusterTrkInterstripPos->fill(theVxdID, side, clInterstripPos);

      //      if(svdClustersTrack.size()%2 != 0)
      //  B2INFO(" Event Number = "<<eventMD->getEvent()<<"  "<<theVxdID<<"."<<side<<" position = "<<svdClustersTrack[cl]->getPosition()<<" charge = "<<svdClustersTrack[cl]->getCharge());

    }
  }

  if (m_Tracks)
    B2DEBUG(1, "%%%%%%%% NEW EVENT,  number of Tracks =  " << m_Tracks.getEntries());


  //reco digits
  if (m_svdRecos.isValid()) {
    for (int digi = 0 ; digi < m_svdRecos.getEntries(); digi++) {

      VxdID::baseType theVxdID = (VxdID::baseType)m_svdRecos[digi]->getSensorID();
      int side = m_svdRecos[digi]->isUStrip();
      int cellID = m_svdRecos[digi]->getCellID();

      float thisNoise = m_NoiseCal.getNoiseInElectrons(theVxdID, side, cellID);

      h_recoNoise->fill(theVxdID, side, thisNoise);
      h_recoCharge->fill(theVxdID, side, m_svdRecos[digi]->getCharge() / 1000.);
      h_recoEnergy->fill(theVxdID, side, m_svdRecos[digi]->getCharge()*c_eTOkeV);
      h_recoTime->fill(theVxdID, side, m_svdRecos[digi]->getTime());
    }
  }

  //clusters  NOT related to tracks
  for (int cl = 0 ; cl < m_svdClusters.getEntries(); cl++) {

    float clCharge = m_svdClusters[cl]->getCharge();
    float clEnergy = m_svdClusters[cl]->getCharge() * c_eTOkeV;
    int clSize = m_svdClusters[cl]->getSize();
    float clTime = m_svdClusters[cl]->getClsTime();
    float clSN = m_svdClusters[cl]->getSNR();

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(m_svdClusters[cl]);

    bool isAssigned = false;
    //    for(int r = 0; r<(int)theRC.size(); r++){
    if (theRC.size() > 0) {
      RelationVector<Track> theT = DataStore::getRelationsWithObj<Track>(theRC[0]);
      if (theT.size() > 0)
        isAssigned = true;
      //    }
    }

    if (isAssigned)
      continue;

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdClusters[cl]->getSensorID();
    int side = m_svdClusters[cl]->isUCluster();

    h_clusterCharge->fill(theVxdID, side, clCharge / 1000.);
    h_clusterSize->fill(theVxdID, side, clSize);
    h_clusterSNR->fill(theVxdID, side, clSN);
    h_clusterEnergy->fill(theVxdID, side, clEnergy);
    h_clusterTime->fill(theVxdID, side, clTime);
  }
}


void SVDB4CommissioningPlotsModule::endRun()
{
  B2INFO("SVDB4CommissioningPlotsModule::endRun(), writing the histograms");

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;

    TDirectory* dir_track = oldDir->mkdir("tracks");
    dir_track->cd();
    m_nTracks->Write();
    m_Pvalue->Write();
    m_mom->Write();
    m_nSVDhits->Write();

    TDirectory* dir_reco = oldDir->mkdir("recoDigits");
    TDirectory* dir_clusterAssigned = oldDir->mkdir("clusters_assigned");
    TDirectory* dir_clusterNotAssigned = oldDir->mkdir("clusters_not_assigned");

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      TString layerName = Form("layer%d", layer.getLayerNumber());
      TDirectory* dir_reco_layer = dir_reco->mkdir(layerName.Data());
      TDirectory* dir_clusterAssigned_layer = dir_clusterAssigned->mkdir(layerName.Data());
      TDirectory* dir_clusterNotAssigned_layer = dir_clusterNotAssigned->mkdir(layerName.Data());
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {

            dir_reco_layer->cd();
            (h_recoCharge->getHistogram(sensor, view))->Write();
            (h_recoEnergy->getHistogram(sensor, view))->Write();
            (h_recoTime->getHistogram(sensor, view))->Write();
            (h_recoNoise->getHistogram(sensor, view))->Write();
            dir_clusterAssigned_layer->cd();
            (h_clusterTrkCharge->getHistogram(sensor, view))->Write();
            (h_clusterTrkSNR->getHistogram(sensor, view))->Write();
            (h_clusterTrkSize->getHistogram(sensor, view))->Write();
            (h_clusterTrkEnergy->getHistogram(sensor, view))->Write();
            (h_clusterTrkTime->getHistogram(sensor, view))->Write();
            (h_clusterTrkInterstripPos->getHistogram(sensor, view))->Write();
            dir_clusterNotAssigned_layer->cd();
            (h_clusterCharge->getHistogram(sensor, view))->Write();
            (h_clusterSNR->getHistogram(sensor, view))->Write();
            (h_clusterSize->getHistogram(sensor, view))->Write();
            (h_clusterEnergy->getHistogram(sensor, view))->Write();
            (h_clusterTime->getHistogram(sensor, view))->Write();
          }
    }

    m_rootFilePtr->Close();

  }

}



void SVDB4CommissioningPlotsModule::terminate()
{
}
