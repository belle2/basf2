#include <tracking/modules/hitToTrueXP/hitToTrueXPModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/hitToTrueXPDerivate.h>


// #include <cstdio>
// #include <stdio.h>
// #include <stdlib.h>
// #include "TFile.h"
// #include <TCanvas.h>
// #include <iostream>
// #include <fstream>
#include <string>
// #include <TRandom3.h>
// #include <TMinuit.h>
// #include "TH1.h"
// #include "TF1.h"
// #include "TH2.h"
// #include "TF2.h"
// #include "TGraphErrors.h"
// #include "TStyle.h"
#include "TMath.h"
// #include "TMatrixDSym.h"
// #include "TFitResult.h"
// #include "TLegend.h"
// #include "TColor.h"
// #include "TPaveText.h"
// #include "TPaveStats.h"
// #include "TGraphAsymmErrors.h"
// #include "TMacro.h"
// #include "THStack.h"
// #include "TLegendEntry.h"
// #include "TDatime.h"
// #include "TString.h"
// #include "TStyle.h"
// #include "TLatex.h"
// #include "TRandom3.h"
// #include "TGraphPainter.h"

#include <algorithm>
#include <functional>

using namespace Belle2;

REG_MODULE(hitToTrueXP)

hitToTrueXPModule::hitToTrueXPModule() : Module()
{
  setDescription("This module builds a ttree with true hit informations (momentum, position). Track parameters hit per hit are accessible too.");

}


hitToTrueXPModule::~hitToTrueXPModule()
{
}



void hitToTrueXPModule::initialize()
{

//iniziialize of useful store array and relations
  StoreArray<SVDDigit> storeDigits("");
  StoreArray<SVDCluster> storeClusters("");
  StoreArray<SVDTrueHit> storeTrueHits("");
  StoreArray<MCParticle> storeMCParticles("");
  StoreArray<RecoTrack> recoTracks("");
  // StoreArray<TrackCandidates> TrackCandidates("");


  storeDigits.isRequired();
  storeClusters.isRequired();
  storeTrueHits.isRequired();
  storeMCParticles.isRequired();
  recoTracks.isRequired();
  // TrackCandidates.isRequired();


  RelationArray relClusterDigits(storeClusters, storeDigits);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);
  RelationArray recoTracksToMCParticles(recoTracks , storeMCParticles);
  // RelationArray TrackCandidatesToMCParticles(TrackCandidates , storeMCParticles );



//inizialize output TFile (ttree, with own-class (hittoTrueXP) branch)
//nb: is not possibile to completely access to entries of this tree using external scripts
  m_outputFile = new TFile("TFile_hitToTrueXP.root", "RECREATE");
  m_tree = new TTree("TTree_hitToTrueXP", "TTree_hitToTrueXP");


  m_tree->Branch("hitToTrueXP", &m_hitToTrueXP);
  m_tree->Branch("trackNumber", &m_trackNumber);
  m_tree->Branch("eventNumber", &m_eventNumber);
  m_tree->Branch("numberHitPerTrack", &m_numberHitPerTrack);
  //m_tree->Branch("numberHitPerEvent", &m_numberHitPerEvent);
  track_iterator = 0;
  event_iterator = 0;

  //-------------------------------------------------------------------------------------------------//
  //------------------------------------selected Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  m_outputFileSel = new TFile("TFile_hitToTrueXPSel.root", "RECREATE");
  m_treeSel = new TTree("TTree_hitToTrueXPSel", "TTree_hitToTrueXPSel");


  m_treeSel->Branch("hitToTrueXP", &m_hitToTrueXPSel);
  m_treeSel->Branch("trackNumber", &m_trackNumberSel);
  m_treeSel->Branch("eventNumber", &m_eventNumberSel);
  m_treeSel->Branch("numberHitPerTrack", &m_numberHitPerTrackSel);






  //-------------------------------------------------------------------------------------------------//
  //------------------------------------tight selected Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  m_outputFileTiSel = new TFile("TFile_hitToTrueXPTiSel.root", "RECREATE");
  m_treeTiSel = new TTree("TTree_hitToTrueXPTiSel", "TTree_hitToTrueXPTiSel");


  m_treeTiSel->Branch("hitToTrueXP", &m_hitToTrueXPTiSel);
  m_treeTiSel->Branch("trackNumber", &m_trackNumberTiSel);
  m_treeTiSel->Branch("eventNumber", &m_eventNumberTiSel);
  m_treeTiSel->Branch("numberHitPerTrack", &m_numberHitPerTrackTiSel);




//-------------------------------------------------------------------------------------------------//
//------------------------------------External Tree creation--------------------------------------//
//-------------------------------------------------------------------------------------------------//

//output tree for complete external use (same datas, but using only root default classes)
  m_outputFileExt = new TFile("TFile_hitToTrueXP_ext.root", "RECREATE");
  m_treeExt = new TTree("TTree_hitToTrueXP_ext", "TTree_hitToTrueXP_ext");


  m_treeExt->Branch("positionEntryX", &e_positionEntryX);
  m_treeExt->Branch("positionEntryY", &e_positionEntryY);
  m_treeExt->Branch("positionEntryZ", &e_positionEntryZ);
  m_treeExt->Branch("momentumEntryX", &e_momentumEntryX);
  m_treeExt->Branch("momentumEntryY", &e_momentumEntryY);
  m_treeExt->Branch("momentumEntryZ", &e_momentumEntryZ);
  m_treeExt->Branch("positionLocalEntryX", &e_positionLocalEntryX);
  m_treeExt->Branch("positionLocalEntryY", &e_positionLocalEntryY);
  m_treeExt->Branch("positionLocalEntryZ", &e_positionLocalEntryZ);
  m_treeExt->Branch("PDGID", &e_PDGID);
  m_treeExt->Branch("position0X", &e_position0X);
  m_treeExt->Branch("position0Y", &e_position0Y);
  m_treeExt->Branch("position0Z", &e_position0Z);
  m_treeExt->Branch("momentum0X", &e_momentum0X);
  m_treeExt->Branch("momentum0Y", &e_momentum0Y);
  m_treeExt->Branch("momentum0Z", &e_momentum0Z);
  m_treeExt->Branch("time", &e_time);
  m_treeExt->Branch("sensorSensor", &e_sensorSensor);
  m_treeExt->Branch("sensorLayer", &e_sensorLayer);
  m_treeExt->Branch("sensorLadder", &e_sensorLadder);
  m_treeExt->Branch("reconstructed", &e_reconstructed);
  m_treeExt->Branch("clusterU", &e_clusterU);
  m_treeExt->Branch("clusterV", &e_clusterV);
  m_treeExt->Branch("charge", &e_charge);
  m_treeExt->Branch("trackNumber", &e_trackNumber);
  m_treeExt->Branch("eventNumber", &e_eventNumber);
  m_treeExt->Branch("numberHitPerTrack", &e_numberHitPerTrack);
  m_treeExt->Branch("omegaEntry", &e_omegaEntry);
  m_treeExt->Branch("omega0", &e_omega0);
  m_treeExt->Branch("d0Entry", &e_d0Entry);
  m_treeExt->Branch("d00", &e_d00);
  m_treeExt->Branch("phi0Entry", &e_phi0Entry);
  m_treeExt->Branch("phi00", &e_phi00);
  m_treeExt->Branch("z0Entry", &e_z0Entry);
  m_treeExt->Branch("z00", &e_z00);
  m_treeExt->Branch("tanlambdaEntry", &e_tanlambdaEntry);
  m_treeExt->Branch("tanlambda0", &e_tanlambda0);
  m_treeExt->Branch("primary", &e_primary);



}

void hitToTrueXPModule::beginRun() {}

void hitToTrueXPModule::event()
{
  StoreArray<SVDDigit> SVDDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;
  StoreArray<MCParticle> MCParticles;

  StoreArray<RecoTrack> recoTracks;

  m_eventNumber = event_iterator;
  e_eventNumber = event_iterator; //------------External Tree---------//
  event_iterator = event_iterator + 1;


  for (const MCParticle& particle : MCParticles) {
    int hit_iterator = 0;
    e_primary = particle.getStatus();
    for (const SVDCluster& cluster : particle.getRelationsFrom<SVDCluster>()) {
      for (const SVDTrueHit& hit : cluster.getRelationsTo<SVDTrueHit>()) {
        hit_iterator++;
        VxdID trueHitSensorID = hit.getSensorID();
        const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().getSensorInfo(trueHitSensorID);
        hitToTrueXPDerivate entry(hit, cluster, particle, sensorInfo);
        int NClusterU = 0;
        int NClusterV = 0;
        for (SVDCluster Ncluster : hit.getRelationsFrom<SVDCluster>()) {
          if (Ncluster.isUCluster()) NClusterU++;
          else NClusterV++;
        }
        entry.setClusterU(NClusterU);
        entry.setClusterV(NClusterV);

        bool isReconstructed(false);
        for (const RecoTrack& aRecoTrack : particle.getRelationsFrom<RecoTrack>())
          isReconstructed |= aRecoTrack.hasSVDHits();
        entry.setReconstructed(isReconstructed);

        set_hitToTrueXP.insert(entry);




      }
    }
    m_trackNumber = track_iterator;
    e_trackNumber = track_iterator; //----------------External Tree ----------------//
    track_iterator = track_iterator + 1;

    for (auto element : set_hitToTrueXP) {
      m_hitToTrueXP.push_back(element);

      //-----------------External Tree ---------------------------- //
      e_positionEntryX.push_back(element.m_positionEntry.x());
      e_positionEntryY.push_back(element.m_positionEntry.y());
      e_positionEntryZ.push_back(element.m_positionEntry.z());
      e_momentumEntryX.push_back(element.m_momentumEntry.x());
      e_momentumEntryY.push_back(element.m_momentumEntry.y());
      e_momentumEntryZ.push_back(element.m_momentumEntry.z());
      e_positionLocalEntryX.push_back(element.m_positionLocalEntry.x());
      e_positionLocalEntryY.push_back(element.m_positionLocalEntry.y());
      e_positionLocalEntryZ.push_back(element.m_positionLocalEntry.z());
      e_PDGID.push_back(element.m_PDGID);
      e_position0X.push_back(element.m_position0.x());
      e_position0Y.push_back(element.m_position0.y());
      e_position0Z.push_back(element.m_position0.z());
      e_momentum0X.push_back(element.m_momentum0.x());
      e_momentum0Y.push_back(element.m_momentum0.y());
      e_momentum0Z.push_back(element.m_momentum0.z());
      e_time.push_back(element.m_time);
      e_sensorSensor.push_back(element.m_sensorSensor);
      e_sensorLayer.push_back(element.m_sensorLayer);
      e_sensorLadder.push_back(element.m_sensorLadder);
      e_reconstructed.push_back(element.m_reconstructed);
      e_clusterU.push_back(element.m_clusterU);
      e_clusterV.push_back(element.m_clusterV);
      e_charge.push_back(element.m_charge);
      e_omegaEntry.push_back(element.getOmegaEntry());
      e_omega0.push_back(element.getOmega0());
      e_d0Entry.push_back(element.getD0Entry());
      e_d00.push_back(element.getD00());
      e_phi0Entry.push_back(element.getPhi0Entry());
      e_phi00.push_back(element.getPhi00());
      e_z0Entry.push_back(element.getZ0Entry());
      e_z00.push_back(element.getZ00());
      e_tanlambdaEntry.push_back(element.getTanLambdaEntry());
      e_tanlambda0.push_back(element.getTanLambda0());
    }
    m_numberHitPerTrack = m_hitToTrueXP.size();
    e_numberHitPerTrack = m_hitToTrueXP.size(); //-------external tree--------------//


    m_tree->Fill();

    //-------external tree--------------//
    m_treeExt->Fill();
    e_positionEntryX.clear();
    e_positionEntryY.clear();
    e_positionEntryZ.clear();
    e_momentumEntryX.clear();
    e_momentumEntryY.clear();
    e_momentumEntryZ.clear();
    e_positionLocalEntryX.clear();
    e_positionLocalEntryY.clear();
    e_positionLocalEntryZ.clear();
    e_PDGID.clear();
    e_position0X.clear();
    e_position0Y.clear();
    e_position0Z.clear();
    e_momentum0X.clear();
    e_momentum0Y.clear();
    e_momentum0Z.clear();
    e_time.clear();
    e_sensorSensor.clear();
    e_sensorLayer.clear();
    e_sensorLadder.clear();
    e_reconstructed.clear();
    e_clusterU.clear();
    e_clusterV.clear();
    e_charge.clear();
    e_omegaEntry.clear();
    e_omega0.clear();
    e_d0Entry.clear();
    e_d00.clear();
    e_phi0Entry.clear();
    e_phi00.clear();
    e_z0Entry.clear();
    e_z00.clear();
    e_tanlambdaEntry.clear();
    e_tanlambda0.clear();



    //-------------------------------------------------------------------------------------------------//
    //------------------------------------Selected Tree creation--------------------------------------//
    //-------------------------------------------------------------------------------------------------//
    //this selecation take tracks that has at least one hit on each layer, and make 4-hit-trakcs selecting first hit found on each layer.
    int f3 = 0, f4 = 0, f5 = 0, f6 = 0;
    int layer_flag = 0;
    int j = 0;
    int brk = 0;
    std::vector<hitToTrueXP> temp_hitToTrueXP;
    while (j < (int)m_hitToTrueXP.size() && brk == 0) {
      if (m_hitToTrueXP[j].m_sensorLayer == 3) {
        f3 = 1;
        temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
        j++;
        while (j < (int)m_hitToTrueXP.size() && brk == 0) {
          if (m_hitToTrueXP[j].m_sensorLayer == 4) {
            f4 = 1;
            temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
            j++;
            while (j < (int)m_hitToTrueXP.size() && brk == 0) {
              if (m_hitToTrueXP[j].m_sensorLayer == 5) {
                f5 = 1;
                temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
                j++;
                while (j < (int)m_hitToTrueXP.size() && brk == 0) {
                  if (m_hitToTrueXP[j].m_sensorLayer == 6) {
                    f6 = 1;
                    temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
                    brk = 1;
                    j++;
                  }
                  j++;
                }
              }
              j++;
            }
          }
          j++;
        }
      }
      j++;
    }
    layer_flag = f3 * f4 * f5 * f6;
    if (layer_flag != 0) {
      int k;
      for (k = 0; k < 4; k = k + 1) {
        m_hitToTrueXPSel.push_back(temp_hitToTrueXP[k]);
      }
      //  if(m_numberHitPerTrack>10){
      //    int k=0;
      //     for(k=0; k<4; k=k+1){
      //       m_hitToTrueXPSel.push_back(m_hitToTrueXP[k]);
      //      }}
      m_trackNumberSel =  m_trackNumber;
      m_eventNumberSel = m_eventNumber;
      m_numberHitPerTrackSel = m_hitToTrueXPSel.size();
      m_treeSel->Fill();
      m_hitToTrueXPSel.erase(m_hitToTrueXPSel.begin(), m_hitToTrueXPSel.end());
    }



    //-------------------------------------------------------------------------------------------------//
    //------------------------------------Tight Selected Tree creation--------------------------------------//
    //-------------------------------------------------------------------------------------------------//
    //this selecation take tracks that has exaclty one hit on each layer, so cut away overlap from selected tree
    if (m_numberHitPerTrack == 4 && m_hitToTrueXP[0].m_sensorLayer == 3 && m_hitToTrueXP[1].m_sensorLayer == 4
        && m_hitToTrueXP[2].m_sensorLayer == 5 && m_hitToTrueXP[3].m_sensorLayer == 6) {
      int h = 0;
      for (h = 0; h < 4; h = h + 1) {
        m_hitToTrueXPTiSel.push_back(m_hitToTrueXP[h]);
      }
      m_trackNumberTiSel =  m_trackNumber;
      m_eventNumberTiSel = m_eventNumber;
      m_numberHitPerTrackTiSel = m_hitToTrueXPTiSel.size();
      m_treeTiSel->Fill();
      m_hitToTrueXPTiSel.erase(m_hitToTrueXPTiSel.begin(), m_hitToTrueXPTiSel.end());
    }



    m_hitToTrueXP.erase(m_hitToTrueXP.begin(), m_hitToTrueXP.end());
    set_hitToTrueXP.clear();
  }
}



void hitToTrueXPModule::endRun()
{
  m_outputFile->cd();
  m_tree->Write();
  m_outputFile->Close();

  //-------------------------------------------------------------------------------------------------//
  //------------------------------------selected Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  m_outputFileSel->cd();
  m_treeSel->Write();
  m_outputFileSel->Close();



  //-------------------------------------------------------------------------------------------------//
  //------------------------------------tight selected Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  m_outputFileTiSel->cd();
  m_treeTiSel->Write();
  m_outputFileTiSel->Close();


  //-------------------------------------------------------------------------------------------------//
  //------------------------------------external Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  m_outputFileExt->cd();
  m_treeExt->Write();
  m_outputFileExt->Close();
}

void hitToTrueXPModule::terminate() {}













//TFile *my_file = TFile::Open("TFile_hitToTrueXP.root");
//TTree *data_tree = (TTree*) my_file->Get("TTree_hitToTrueXP");

// int m_trackNumber, m_eventNumber, m_numberHitPerTrack;
// std::vector<Belle2::hitToTrueXP> m_hitToTrueXP;
// data_tree->SetBranchAddress("hitToTrueXP", &m_hitToTrueXP);
// data_tree->SetBranchAddress("trackNumber", &m_trackNumber);
// data_tree->SetBranchAddress("eventNumber", &m_eventNumber);
// data_tree->SetBranchAddress("numberHitPerTrack", &m_numberHitPerTrack);

// Int_t Nentries = Int_t(m_tree->GetEntries());
//   for(int i=0; i<Nentries; i++){
//      m_tree->GetEntry(i);
//       int f3=0,f4=0,f5=0,f6=0;
//       int layer_flag = 0;
//       int j=0;
//       int brk=0;
//       std::vector<hitToTrueXP> temp_hitToTrueXP;
//       while(j<m_hitToTrueXP.size() && brk==0){
//         if(m_hitToTrueXP[j].m_sensorLayer==3) {
//           f3=1;
//           temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
//           j++;
//           while(j<m_hitToTrueXP.size() && brk==0){
//             if(m_hitToTrueXP[j].m_sensorLayer==4) {
//               f4=1;
//               temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
//               j++;
//               while(j<m_hitToTrueXP.size() && brk==0){
//                 if(m_hitToTrueXP[j].m_sensorLayer==5){
//                   f5=1;
//                   temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
//                   j++;
//                   while(j<m_hitToTrueXP.size() && brk==0){
//                     if(m_hitToTrueXP[j].m_sensorLayer==6){
//                       f6=1;
//                       temp_hitToTrueXP.push_back(m_hitToTrueXP[j]);
//                       brk=1;
//                       j++;
//                     }
//                     j++;
//                   }
//                 }
//                 j++;
//               }
//             }
//             j++;
//           }
//         }
//         j++;
//       }
//       layer_flag = f3*f4*f5*f6;
//       if(layer_flag!=0){
//         for(int k=0; k<4; k=k++){
//           m_hitToTrueXPSel.push_back(temp_hitToTrueXP[k]);
//         }
//         m_trackNumberSel=  m_trackNumber;
//         m_eventNumberSel= m_eventNumber;
//         m_numberHitPerTrackSel= m_numberHitPerTrack;
//       }
//       m_treeSel->Fill();
//       m_hitToTrueXPSel.erase(m_hitToTrueXPSel.begin(),m_hitToTrueXPSel.end());
//     }

//  m_outputFile->Close();
//my_file->Close();
