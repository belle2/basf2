/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmSimHistogrammer/BKLMSimHistogrammerModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/GearDir.h>

#include <bklm/geometry/GeometryPar.h>
#include <bklm/dataobjects/BKLMSimHit.h>

#include <bklm/dataobjects/BKLMSimHitPosition.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <eklm/simulation/FPGAFitter.h>
#include <simulation/dataobjects/SimHitBase.h>

#include <TRandom.h>
#include "TMath.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::bklm;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMSimHistogrammer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMSimHistogrammerModule::BKLMSimHistogrammerModule() : Module()
{

  setDescription("Analyzes bg");

  addParam("filename", m_filename, "Output root filename", string("bg_output.root"));
  addParam("realTime", m_realTime, "Time the simulation corresponds to", float(1.0));


}

BKLMSimHistogrammerModule::~BKLMSimHistogrammerModule()
{
}

void BKLMSimHistogrammerModule::initialize()
{

  hits2D.isRequired();
  simHits.isRequired();


  m_file = new TFile(m_filename.c_str(), "recreate");

  m_hEvt = new TH1D("hEvts", "hEvts", 10001, -1, 10000);

  m_hSimHitPhiRPC = new TH1D("simhitPhiRPC", "simHitPhiRPC", 100, -1 * TMath::Pi(), 1 * TMath::Pi());
  m_hSimHitPhiScinti = new TH1D("simhitPhiScinti", "simHitPhiScinti", 100, -1 * TMath::Pi(), 1 * TMath::Pi());
  m_hSimHitThetaRPC = new TH1D("simhitThetaRPC", "simHitThetaRPC", 100, 0, 1 * TMath::Pi());
  m_hSimHitThetaScinti = new TH1D("simhitThetaScinti", "simHitThetaScinti", 100, 0, 1 * TMath::Pi());

  m_hSimHit_layer = new TH1I("simHitLayer", "simHitLayer", 16, 0, 15);
  m_hSimHit_layer2D = new TH1I("simHitLayer2D", "simHitLayer2D", 16, 0, 15);

  m_bgSourcePerLayer = new TH2D("bgSourcPerLayer", "bgSourcePerLayer", 21, 0, 20, 16, 0, 15);
  m_bgSourcePerLayer2D = new TH2D("bgSourcPerLayer2D", "bgSourcePerLayer2D", 21, 0, 20, 16, 0, 15);
  m_bgSourceVsPhi = new TH2D("bgSourceVsPhi", "bgSourceVsPhi", 100, -1 * TMath::Pi(), TMath::Pi(), 21, 0, 20);
  m_bgSourceVsTheta = new TH2D("bgSourceVsTheta", "bgSourceVsTheta", 100, 0, TMath::Pi(), 21, 0, 20);

  m_bgSource = new TH1D("bgSource", "bgSource", 21, 0, 20);
  m_bgSource2D = new TH1D("bgSource2D", "bgSource2D", 21, 0, 20);

  m_hSimHitPerChannelLayer = new TH2D("posPerChannelLayer", "posPerChannelLayer", 5000, 0, 5000, 16, 0, 15);


  // Force creation and persistence of BKLM output datastores
}

void BKLMSimHistogrammerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("BKLMSimHistogrammer: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun());
}

void BKLMSimHistogrammerModule::event()
{
  //---------------------------------------------
  // Get BKLM hits collection from the data store
  //---------------------------------------------

  int nSimHit = simHits.getEntries();

  //use 1D hits (otherwise too many simHits, also look up background info), only problem might be that the threshold in simulation is not correct


  //  unsigned int d = 0;

  if (m_realTime > 0)
    m_weight = 1.0 / m_realTime;

  cout << "real time " << m_realTime << " weight: " << m_weight << endl;

  int n2DHits = hits2D.getEntries();
  int n1DHits = hits1D.getEntries();
  cout << "we have " << nSimHit << " sim hits " << n1DHits   << " 1D hits " << n2DHits << " 2D hits " << endl;

  //  cout <<"we have " << digits.getEntries() <<" digits " <<endl;

  m_hEvt->Fill(n2DHits, m_weight);
  for (int i = 0; i < hits1D.getEntries(); i++) {
    cout << "looking at 1DHit " << i << endl;
    int scaledTag = -1;
    RelationVector<BKLMDigit> bklmDigits = hits1D[i]->getRelationsTo<BKLMDigit>();
    for (const auto& bklmDigit : bklmDigits) {
      RelationVector<BKLMSimHit> relatedSimHits = bklmDigit.getRelationsWith<BKLMSimHit>();
      for (const auto& simHit : relatedSimHits) {
        auto bgTag = simHit.getBackgroundTag();
        scaledTag = bgTag;
        //other has numeric value of 99
        if (scaledTag > 20)
          scaledTag = 20;
        cout << "scaledTag: " << scaledTag << endl;
        break;



//        switch(bgTag)
//    {
//    case SimHitBase::bg_none:
//      cout <<"this is no bg " <<endl;
//      break;
//    case SimHitBase::bg_Coulomb_LER:
//      cout <<"coulomb ler" <<endl;
//      break;
//    case SimHitBase::bg_Coulomb_HER:
//      cout <<"coulomb her" <<endl;
//      break;
//
//    case SimHitBase::bg_Touschek_LER:
//      cout <<"touschek ler" <<endl;
//      break;
//    case SimHitBase::bg_Touschek_HER:
//      cout <<"touschek her" <<endl;
//      break;
//
//    case SimHitBase::bg_RBB_LER:
//      cout <<"rbb ler" <<endl;
//      break;
//
//    case SimHitBase::bg_RBB_HER:
//      cout <<"rbb her" <<endl;
//      break;
//
//    case SimHitBase::bg_twoPhoton:
//      cout <<" two phot" <<endl;
//      break;
//    case SimHitBase::bg_RBB_gamma:
//      cout <<"rbb gamma" <<endl;
//      break;
//    case SimHitBase::bg_RBB_LER_far:
//      cout <<"rbb ler far" <<endl;
//      break;
//    case SimHitBase::bg_RBB_HER_far:
//      cout <<"rbb her far" <<endl;
//      break;
//    case SimHitBase::bg_Touschek_LER_far:
//      cout <<"touschek ler far" <<endl;
//      break;
//    case SimHitBase::bg_Touschek_HER_far:
//      cout <<"touschek her far" <<endl;
//      break;
//
//    case SimHitBase::bg_SynchRad_LER:
//      cout <<"synchRad ler " <<endl;
//      break;
//    case SimHitBase::bg_SynchRad_HER:
//      cout <<"synchRad her " <<endl;
//      break;
//    case SimHitBase::bg_BHWide_LER:
//      cout <<"bh wide ler " <<endl;
//      break;
//    case SimHitBase::bg_BHWide_HER:
//      cout <<"bh wide her " <<endl;
//      break;
//    case SimHitBase::bg_other:
//      cout <<"bg other " <<endl;
//      break;
//
//    default:
//      {
//        cout <<"this bg code is not allowed! " << bgTag <<endl;
//        break;
//      }
//
//    }
      }
      break;

    }
    int channel = hits1D[i]->isForward() * 840 +  hits1D[i]->getSector() * 105 + hits1D[i]->isPhiReadout() * 1680 +
                  hits1D[i]->getStripAve();
    m_hSimHitPerChannelLayer->Fill(channel, hits1D[i]->getLayer(), m_weight);
    m_bgSource->Fill(scaledTag, m_weight);
    m_bgSourcePerLayer->Fill(scaledTag, hits1D[i]->getLayer(), m_weight);
    cout << "filling layer with tag: " << scaledTag  << " and layer " << hits1D[i]->getLayer() << endl;
  }

  for (int i = 0; i < hits2D.getEntries(); i++) {
    int scaledTag = -1;
    TVector3 gHitPos = hits2D[i]->getGlobalPosition();
    RelationVector<BKLMHit1d> related1DHits = hits2D[i]->getRelationsTo<BKLMHit1d>();
    for (const auto& hit1d : related1DHits) {
      RelationVector<BKLMDigit> bklmDigits = hit1d.getRelationsTo<BKLMDigit>();
      for (const auto& bklmDigit : bklmDigits) {
        RelationVector<BKLMSimHit> relatedSimHits = bklmDigit.getRelationsWith<BKLMSimHit>();
        for (const auto& simHit : relatedSimHits) {
          auto bgTag = simHit.getBackgroundTag();
          scaledTag = bgTag;
          //other has numeric value of 99
          if (scaledTag > 20)
            scaledTag = 20;


          m_bgSourcePerLayer2D->Fill(scaledTag, hits2D[i]->getLayer(), m_weight);
          m_bgSource2D->Fill(scaledTag, m_weight);
          m_bgSourceVsTheta->Fill(gHitPos.Theta(), scaledTag, m_weight);
          m_bgSourceVsPhi->Fill(gHitPos.Phi(), scaledTag, m_weight);
          //one bg source is enough
          break;
        }
        break;
      }
      break;
    }
  }




//  for(int i=0;i<digits.getEntries();i++)
//    {
//      RelationVector<BKLMSimHit> simHits=digits[i]->getRelationsFrom<BKLMSimHit>();
//      cout <<"digit " << i <<" has " << simHits.size() << " sim Hits associated " <<endl;
//    }
//  //    RelationVector<BKLMSimHit> bklmSimHits =        bklmDigits[i4]->getRelationsFrom<BKLMSimHit>();
//
//      for (int i3 = 0; i3 < n1DHits; i3++) {
//        RelationVector<BKLMDigit> bklmDigits =
//          hits1D[i3]->getRelationsTo<BKLMDigit>();
//  cout <<"the 1Dhit " << i3 << "has "<< bklmDigits.size()<< " digits" <<endl;
//        int n4 = bklmDigits.size();
//        for (int i4 = 0; i4 < n4; i4++) {
//          RelationVector<BKLMSimHit> bklmSimHits =
//            bklmDigits[i4]->getRelationsFrom<BKLMSimHit>();
//    cout <<"and this digit is associated with " << bklmSimHits.size()<<" sim hits " <<endl;
//    //          n5 = bklmSimHits.size();
//  }}
//
//


  if (nSimHit == 0) return;
  for (int i = 0; i < n2DHits; i++) {
    BKLMHit2d* hit2D = hits2D[i];
    TVector3 gHitPos = hit2D->getGlobalPosition();
    if (hit2D->inRPC()) {
      m_hSimHitPhiRPC->Fill(gHitPos.Phi(), m_weight);
      m_hSimHitThetaRPC->Fill(gHitPos.Theta(), m_weight);
    } else {
      m_hSimHitPhiScinti->Fill(gHitPos.Phi(), m_weight);
      m_hSimHitThetaScinti->Fill(gHitPos.Theta(), m_weight);
    }



  }

  for (int h = 0; h < nSimHit; ++h) {
    BKLMSimHit* simHit = simHits[h];

    RelationVector<MCParticle> bklmMCParticles = simHit->getRelationsFrom<MCParticle>();
//  cout <<"did " << bklmMCParticles.size() <<" as relation from " <<endl;
    if (bklmMCParticles.size() > 0) {
      cout << "found MC particle!" << endl;
      RelationVector<MCParticle> bklmMCParticlesTo = simHit->getRelationsTo<MCParticle>();
      cout << "got " << bklmMCParticlesTo.size() << " as relation to " << endl;
    }



// //getMomentum, and getProductionVector  getPDG
  }


//
//td::map<int, std::vector<std::pair<int, BKLMSimHit*> > > volIDToSimHits;
//
//
// BKLMSimHit* simHit = simHits[h];
//
// RelationVector<MCParticle> bklmMCParticles =simHit->getRelationsFrom<MCParticle>();
// cout <<"did " << bklmMCParticles.size() <<" as relation from " <<endl;
//
// RelationVector<MCParticle> bklmMCParticlesTo =simHit->getRelationsTo<MCParticle>();
// //    RelationVector<MCParticle> bklmMCParticlesTo =simHit->getRelationsTo<MCParticle>();
// cout <<"got " << bklmMCParticlesTo.size() <<" as relation to " <<endl;
// //getMomentum, and getProductionVector  getPDG
//
//
// //    cout <<" looking at sector: "<< simHit->getSector() << " get layer: " << simHit->getLayer()<<" strip: "<< simHit->getStrip()<<endl;
// m_hSimHit_layer->Fill(simHit->getLayer());
// //    if (simHit->inRPC()) {
// {
//   //get relations
//   RelationVector<BKLMSimHitPosition> simPositionRelation=simHit->getRelationsTo<BKLMSimHitPosition>();
//   cout <<" got " << simPositionRelation.size() <<" pos" <<endl;
//
//
//
//   for(unsigned int iPos=0;iPos<simPositionRelation.size();iPos++)
//  {
//     BKLMSimHitPosition *simPos=simPositionRelation[iPos];
//     if(simPos)
//       cout <<"found simPos: "<< simPos->getPosition().Phi() <<" theta: "<< simPos->getPosition().Theta()<<endl;
//     else
//       cout <<"no sim pos " <<endl;
//  }
//
//    BKLMSimHitPosition* pos = simHit->getRelatedFrom<BKLMSimHitPosition>();
//  if (!pos) {
//    //nothing found, do some error handling here
//    cout <<"no position found .." <<endl;
//  }
//  else
//    {
//      cout <<" found pos " << pos->getPosition().Phi() <<" theta: "<< pos->getPosition().Theta() <<endl;
//    }
//
//
// }
//

}

void BKLMSimHistogrammerModule::endRun()
{
}

void BKLMSimHistogrammerModule::terminate()
{
  m_file->Write();
  m_file->Close();
}
