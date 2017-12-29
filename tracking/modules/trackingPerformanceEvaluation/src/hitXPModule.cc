/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/hitXPModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/hitXPDerivate.h>

#include <string>
#include "TMath.h"
#include <algorithm>
#include <functional>

using namespace Belle2;

REG_MODULE(hitXP)

hitXPModule::hitXPModule() : Module()
{
  setDescription("This module builds a ttree with true hit informations (momentum, position). Track parameters hit per hit are accessible too.");

  /** write validation plots */
  addParam("additionalTree", c_addTree,
           "produce two additional tree with reduced date: selTree (track with at least one hit per layer), tiSelTree (track with one hit per layer)",
           false);

}


hitXPModule::~hitXPModule()
{
}



void hitXPModule::initialize()
{

  /** iniziialize of useful store array and relations */
  StoreArray<SVDDigit> storeDigits("");
  StoreArray<SVDCluster> storeClusters("");
  StoreArray<SVDTrueHit> storeTrueHits("");
  StoreArray<MCParticle> storeMCParticles("");
  StoreArray<RecoTrack> recoTracks("");


  storeDigits.isRequired();
  storeClusters.isRequired();
  storeTrueHits.isRequired();
  storeMCParticles.isRequired();
  recoTracks.isRequired();


  RelationArray relClusterDigits(storeClusters, storeDigits);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);
  RelationArray recoTracksToMCParticles(recoTracks , storeMCParticles);



  /** inizialize output TFile (ttree, with own-class (hitXP) branch)
  * nb: is not possibile to completely access to entries of this tree using
  * external (out of basf2) scripts
  */
  m_outputFile = new TFile("TFile_hitXP.root", "RECREATE");
  m_tree = new TTree("TTree_hitXP", "TTree_hitXP");

  m_tree->Branch("hitXP", &m_hitXP);
  m_tree->Branch("trackNumber", &m_trackNumber);
  m_tree->Branch("eventNumber", &m_eventNumber);
  m_tree->Branch("numberHitPerTrack", &m_numberHitPerTrack);

  m_trackIterator = 0;
  m_eventIterator = 0;

  //-------------------------------------------------------------------------------------------------//
  //------------------------------------selected Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  if (c_addTree) m_outputFileSel = new TFile("TFile_hitXPSel.root", "RECREATE");
  m_treeSel = new TTree("TTree_hitXPSel", "TTree_hitXPSel");

  m_treeSel->Branch("hitXP", &m_hitXPSel);
  m_treeSel->Branch("trackNumber", &m_trackNumberSel);
  m_treeSel->Branch("eventNumber", &m_eventNumberSel);
  m_treeSel->Branch("numberHitPerTrack", &m_numberHitPerTrackSel);


  //-------------------------------------------------------------------------------------------------//
  //------------------------------------tight selected Tree creation--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  if (c_addTree) m_outputFileTiSel = new TFile("TFile_hitXPTiSel.root", "RECREATE");
  m_treeTiSel = new TTree("TTree_hitXPTiSel", "TTree_hitXPTiSel");

  m_treeTiSel->Branch("hitXP", &m_hitXPTiSel);
  m_treeTiSel->Branch("trackNumber", &m_trackNumberTiSel);
  m_treeTiSel->Branch("eventNumber", &m_eventNumberTiSel);
  m_treeTiSel->Branch("numberHitPerTrack", &m_numberHitPerTrackTiSel);


//-------------------------------------------------------------------------------------------------//
//------------------------------------External Tree creation--------------------------------------//
//-------------------------------------------------------------------------------------------------//

  /** output tree for complete external use (same datas, but using only root default classes) */
  m_outputFileExt = new TFile("TFile_hitXP_ext.root", "RECREATE");
  m_treeExt = new TTree("TTree_hitXP_ext", "TTree_hitXP_ext");

  m_treeExt->Branch("positionEntryX", &m_EpositionEntryX);
  m_treeExt->Branch("positionEntryY", &m_EpositionEntryY);
  m_treeExt->Branch("positionEntryZ", &m_EpositionEntryZ);
  m_treeExt->Branch("momentumEntryX", &m_EmomentumEntryX);
  m_treeExt->Branch("momentumEntryY", &m_EmomentumEntryY);
  m_treeExt->Branch("momentumEntryZ", &m_EmomentumEntryZ);
  m_treeExt->Branch("positionLocalEntryX", &m_EpositionLocalEntryX);
  m_treeExt->Branch("positionLocalEntryY", &m_EpositionLocalEntryY);
  m_treeExt->Branch("positionLocalEntryZ", &m_EpositionLocalEntryZ);
  m_treeExt->Branch("PDGID", &m_EPDGID);
  m_treeExt->Branch("position0X", &m_Eposition0X);
  m_treeExt->Branch("position0Y", &m_Eposition0Y);
  m_treeExt->Branch("position0Z", &m_Eposition0Z);
  m_treeExt->Branch("momentum0X", &m_Emomentum0X);
  m_treeExt->Branch("momentum0Y", &m_Emomentum0Y);
  m_treeExt->Branch("momentum0Z", &m_Emomentum0Z);
  m_treeExt->Branch("time", &m_Etime);
  m_treeExt->Branch("sensorSensor", &m_EsensorSensor);
  m_treeExt->Branch("sensorLayer", &m_EsensorLayer);
  m_treeExt->Branch("sensorLadder", &m_EsensorLadder);
  m_treeExt->Branch("reconstructed", &m_Ereconstructed);
  m_treeExt->Branch("clusterU", &m_EclusterU);
  m_treeExt->Branch("clusterV", &m_EclusterV);
  m_treeExt->Branch("charge", &m_Echarge);
  m_treeExt->Branch("trackNumber", &m_EtrackNumber);
  m_treeExt->Branch("eventNumber", &m_EeventNumber);
  m_treeExt->Branch("numberHitPerTrack", &m_EnumberHitPerTrack);
  m_treeExt->Branch("omegaEntry", &m_EomegaEntry);
  m_treeExt->Branch("omega0", &m_Eomega0);
  m_treeExt->Branch("d0Entry", &m_Ed0Entry);
  m_treeExt->Branch("d00", &m_Ed00);
  m_treeExt->Branch("phi0Entry", &m_Ephi0Entry);
  m_treeExt->Branch("phi00", &m_Ephi00);
  m_treeExt->Branch("z0Entry", &m_Ez0Entry);
  m_treeExt->Branch("z00", &m_Ez00);
  m_treeExt->Branch("tanlambdaEntry", &m_EtanlambdaEntry);
  m_treeExt->Branch("tanlambda0", &m_Etanlambda0);
  m_treeExt->Branch("primary", &m_Eprimary);

}

void hitXPModule::beginRun() {}

void hitXPModule::event()
{
  StoreArray<SVDDigit> SVDDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;
  StoreArray<MCParticle> MCParticles;

  StoreArray<RecoTrack> recoTracks;

  m_eventNumber = m_eventIterator;
  m_EeventNumber = m_eventIterator; //------------External Tree---------//
  m_eventIterator = m_eventIterator + 1;


  for (const MCParticle& particle : MCParticles) {
    int hit_iterator = 0;
    m_Eprimary = particle.getStatus();
    // for (const SVDCluster& cluster : particle.getRelationsFrom<SVDCluster>()) {
    //   for (const SVDTrueHit& hit : cluster.getRelationsTo<SVDTrueHit>()) {
    for (const SVDTrueHit& hit : particle.getRelationsTo<SVDTrueHit>()) {
      if (hit.getRelationsFrom<SVDCluster>().size() > 0) {
        hit_iterator++;
        VxdID trueHitSensorID = hit.getSensorID();
        const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().getSensorInfo(trueHitSensorID);
        const SVDCluster* cluster = hit.getRelationsFrom<SVDCluster>()[0];
        hitXPDerivate entry(hit, *cluster, particle, sensorInfo);
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
        m_hitXPSet.insert(entry);
      }
    }
    //}
    //}
    m_trackNumber = m_trackIterator;
    m_EtrackNumber = m_trackIterator; //----------------External Tree ----------------//
    m_trackIterator = m_trackIterator + 1;

    for (auto element : m_hitXPSet) {
      m_hitXP.push_back(element);

      //-----------------External Tree ---------------------------- //
      m_EpositionEntryX.push_back(element.m_positionEntry.x());
      m_EpositionEntryY.push_back(element.m_positionEntry.y());
      m_EpositionEntryZ.push_back(element.m_positionEntry.z());
      m_EmomentumEntryX.push_back(element.m_momentumEntry.x());
      m_EmomentumEntryY.push_back(element.m_momentumEntry.y());
      m_EmomentumEntryZ.push_back(element.m_momentumEntry.z());
      m_EpositionLocalEntryX.push_back(element.m_positionLocalEntry.x());
      m_EpositionLocalEntryY.push_back(element.m_positionLocalEntry.y());
      m_EpositionLocalEntryZ.push_back(element.m_positionLocalEntry.z());
      m_EPDGID.push_back(element.m_PDGID);
      m_Eposition0X.push_back(element.m_position0.x());
      m_Eposition0Y.push_back(element.m_position0.y());
      m_Eposition0Z.push_back(element.m_position0.z());
      m_Emomentum0X.push_back(element.m_momentum0.x());
      m_Emomentum0Y.push_back(element.m_momentum0.y());
      m_Emomentum0Z.push_back(element.m_momentum0.z());
      m_Etime.push_back(element.m_time);
      m_EsensorSensor.push_back(element.m_sensorSensor);
      m_EsensorLayer.push_back(element.m_sensorLayer);
      m_EsensorLadder.push_back(element.m_sensorLadder);
      m_Ereconstructed.push_back(element.m_reconstructed);
      m_EclusterU.push_back(element.m_clusterU);
      m_EclusterV.push_back(element.m_clusterV);
      m_Echarge.push_back(element.m_charge);
      m_EomegaEntry.push_back(element.getOmegaEntry());
      m_Eomega0.push_back(element.getOmega0());
      m_Ed0Entry.push_back(element.getD0Entry());
      m_Ed00.push_back(element.getD00());
      m_Ephi0Entry.push_back(element.getPhi0Entry());
      m_Ephi00.push_back(element.getPhi00());
      m_Ez0Entry.push_back(element.getZ0Entry());
      m_Ez00.push_back(element.getZ00());
      m_EtanlambdaEntry.push_back(element.getTanLambdaEntry());
      m_Etanlambda0.push_back(element.getTanLambda0());
    }
    m_numberHitPerTrack = m_hitXP.size();
    m_EnumberHitPerTrack = m_hitXP.size(); //-------external tree--------------//


    m_tree->Fill();

    //-------external tree--------------//
    m_treeExt->Fill();
    m_EpositionEntryX.clear();
    m_EpositionEntryY.clear();
    m_EpositionEntryZ.clear();
    m_EmomentumEntryX.clear();
    m_EmomentumEntryY.clear();
    m_EmomentumEntryZ.clear();
    m_EpositionLocalEntryX.clear();
    m_EpositionLocalEntryY.clear();
    m_EpositionLocalEntryZ.clear();
    m_EPDGID.clear();
    m_Eposition0X.clear();
    m_Eposition0Y.clear();
    m_Eposition0Z.clear();
    m_Emomentum0X.clear();
    m_Emomentum0Y.clear();
    m_Emomentum0Z.clear();
    m_Etime.clear();
    m_EsensorSensor.clear();
    m_EsensorLayer.clear();
    m_EsensorLadder.clear();
    m_Ereconstructed.clear();
    m_EclusterU.clear();
    m_EclusterV.clear();
    m_Echarge.clear();
    m_EomegaEntry.clear();
    m_Eomega0.clear();
    m_Ed0Entry.clear();
    m_Ed00.clear();
    m_Ephi0Entry.clear();
    m_Ephi00.clear();
    m_Ez0Entry.clear();
    m_Ez00.clear();
    m_EtanlambdaEntry.clear();
    m_Etanlambda0.clear();



    //-------------------------------------------------------------------------------------------------//
    //------------------------------------Selected Tree creation--------------------------------------//
    //-------------------------------------------------------------------------------------------------//
    /** this selecation take tracks that has at least one hit on each layer, and make 4-hit-trakcs selecting first hit found on each layer. */
    int f3 = 0, f4 = 0, f5 = 0, f6 = 0;
    int layer_flag = 0;
    int j = 0;
    int brk = 0;
    std::vector<hitXP> temp_hitXP;
    while (j < (int)m_hitXP.size() && brk == 0) {
      if (m_hitXP[j].m_sensorLayer == 3) {
        f3 = 1;
        temp_hitXP.push_back(m_hitXP[j]);
        j++;
        while (j < (int)m_hitXP.size() && brk == 0) {
          if (m_hitXP[j].m_sensorLayer == 4) {
            f4 = 1;
            temp_hitXP.push_back(m_hitXP[j]);
            j++;
            while (j < (int)m_hitXP.size() && brk == 0) {
              if (m_hitXP[j].m_sensorLayer == 5) {
                f5 = 1;
                temp_hitXP.push_back(m_hitXP[j]);
                j++;
                while (j < (int)m_hitXP.size() && brk == 0) {
                  if (m_hitXP[j].m_sensorLayer == 6) {
                    f6 = 1;
                    temp_hitXP.push_back(m_hitXP[j]);
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
        m_hitXPSel.push_back(temp_hitXP[k]);
      }
      m_trackNumberSel =  m_trackNumber;
      m_eventNumberSel = m_eventNumber;
      m_numberHitPerTrackSel = m_hitXPSel.size();
      m_treeSel->Fill();
      m_hitXPSel.erase(m_hitXPSel.begin(), m_hitXPSel.end());
    }



    //-------------------------------------------------------------------------------------------------//
    //------------------------------------Tight Selected Tree creation--------------------------------------//
    //-------------------------------------------------------------------------------------------------//
    /** this selecation take tracks that has exaclty one hit on each layer, so cut away overlap from selected tree */
    if (m_numberHitPerTrack == 4 && m_hitXP[0].m_sensorLayer == 3 && m_hitXP[1].m_sensorLayer == 4
        && m_hitXP[2].m_sensorLayer == 5 && m_hitXP[3].m_sensorLayer == 6) {
      int h = 0;
      for (h = 0; h < 4; h = h + 1) {
        m_hitXPTiSel.push_back(m_hitXP[h]);
      }
      m_trackNumberTiSel =  m_trackNumber;
      m_eventNumberTiSel = m_eventNumber;
      m_numberHitPerTrackTiSel = m_hitXPTiSel.size();
      m_treeTiSel->Fill();
      m_hitXPTiSel.erase(m_hitXPTiSel.begin(), m_hitXPTiSel.end());
    }
    m_hitXP.erase(m_hitXP.begin(), m_hitXP.end());
    m_hitXPSet.clear();
  }
}



void hitXPModule::endRun()
{
  m_outputFile->cd();
  m_tree->Write();
  m_outputFile->Close();

  if (c_addTree) {
    //-------------------------------------------------------------------------------------------------//
    //------------------------------------selected Tree storage--------------------------------------//
    //-------------------------------------------------------------------------------------------------//
    m_outputFileSel->cd();
    m_treeSel->Write();
    m_outputFileSel->Close();


    //-------------------------------------------------------------------------------------------------//
    //------------------------------------tight selected Tree storage--------------------------------------//
    //-------------------------------------------------------------------------------------------------//
    m_outputFileTiSel->cd();
    m_treeTiSel->Write();
    m_outputFileTiSel->Close();
  }


  //-------------------------------------------------------------------------------------------------//
  //------------------------------------external Tree storage--------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  m_outputFileExt->cd();
  m_treeExt->Write();
  m_outputFileExt->Close();
}

void hitXPModule::terminate() {}
