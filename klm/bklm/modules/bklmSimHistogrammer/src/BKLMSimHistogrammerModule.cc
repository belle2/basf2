/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/modules/bklmSimHistogrammer/BKLMSimHistogrammerModule.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMSimHit.h>
#include <klm/dataobjects/KLMDigit.h>

/* Belle 2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/* ROOT headers. */
#include <TMath.h>

/* C++ headers. */
#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMSimHistogrammer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMSimHistogrammerModule::BKLMSimHistogrammerModule() : Module(),
  m_hSimHitPerChannelLayer(nullptr),
  m_hEvt(nullptr),
  m_hSimHitPhiRPC(nullptr),
  m_bgSourcePerLayer(nullptr),
  m_bgSourcePerLayer2D(nullptr),
  m_bgSourceVsPhi(nullptr),
  m_bgSourceVsTheta(nullptr),
  m_bgSource(nullptr),
  m_bgSource2D(nullptr),
  m_hSimHitPhiScinti(nullptr),
  m_hSimHitThetaRPC(nullptr),
  m_hSimHitThetaScinti(nullptr),
  m_hSimHit_layer(nullptr),
  m_hSimHit_layer2D(nullptr),
  m_hSimHitThetaPhiRPC(nullptr),
  m_hSimHitThetaPhiScinti(nullptr),
  m_file(nullptr),
  m_weight(0.0)
{

  setDescription("Analyzes bg");

  addParam("filename", m_filename, "Output root filename", std::string("bg_output.root"));
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

  std::cout << "real time " << m_realTime << " weight: " << m_weight << std::endl;

  int n2DHits = hits2D.getEntries();
  int n1DHits = hits1D.getEntries();
  std::cout << "we have " << nSimHit << " sim hits " << n1DHits   << " 1D hits " << n2DHits << " 2D hits " << std::endl;

  //  std::cout <<"we have " << digits.getEntries() <<" digits " <<std::endl;

  m_hEvt->Fill(n2DHits, m_weight);
  for (int i = 0; i < hits1D.getEntries(); i++) {
    std::cout << "looking at 1DHit " << i << std::endl;
    int scaledTag = -1;
    RelationVector<KLMDigit> bklmDigits = hits1D[i]->getRelationsTo<KLMDigit>();
    for (const auto& bklmDigit : bklmDigits) {
      RelationVector<BKLMSimHit> relatedSimHits = bklmDigit.getRelationsWith<BKLMSimHit>();
      for (const auto& simHit : relatedSimHits) {
        auto bgTag = simHit.getBackgroundTag();
        scaledTag = bgTag;
        //other has numeric value of 99
        if (scaledTag > 20)
          scaledTag = 20;
        std::cout << "scaledTag: " << scaledTag << std::endl;
        break;
      }
      break;

    }
    int channel = hits1D[i]->getSection() * 840 +  hits1D[i]->getSector() * 105 + hits1D[i]->isPhiReadout() * 1680 +
                  hits1D[i]->getStripAve();
    m_hSimHitPerChannelLayer->Fill(channel, hits1D[i]->getLayer(), m_weight);
    m_bgSource->Fill(scaledTag, m_weight);
    m_bgSourcePerLayer->Fill(scaledTag, hits1D[i]->getLayer(), m_weight);
    std::cout << "filling layer with tag: " << scaledTag  << " and layer " << hits1D[i]->getLayer() << std::endl;
  }

  for (int i = 0; i < hits2D.getEntries(); i++) {
    if (hits2D[i]->getSubdetector() != KLMElementNumbers::c_BKLM)
      continue;
    int scaledTag = -1;
    ROOT::Math::XYZVector gHitPos = hits2D[i]->getPosition();
    RelationVector<BKLMHit1d> related1DHits = hits2D[i]->getRelationsTo<BKLMHit1d>();
    for (const auto& hit1d : related1DHits) {
      RelationVector<KLMDigit> bklmDigits = hit1d.getRelationsTo<KLMDigit>();
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
  if (nSimHit == 0)
    return;
  for (int i = 0; i < n2DHits; i++) {
    KLMHit2d* hit2D = hits2D[i];
    ROOT::Math::XYZVector gHitPos = hit2D->getPosition();
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
    if (bklmMCParticles.size() > 0) {
      std::cout << "found MC particle!" << std::endl;
      RelationVector<MCParticle> bklmMCParticlesTo = simHit->getRelationsTo<MCParticle>();
      std::cout << "got " << bklmMCParticlesTo.size() << " as relation to " << std::endl;
    }
  }
}

void BKLMSimHistogrammerModule::endRun()
{
}

void BKLMSimHistogrammerModule::terminate()
{
  m_file->Write();
  m_file->Close();
}
